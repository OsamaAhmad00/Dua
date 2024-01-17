#include <ModuleCompiler.hpp>
#include <parsing/ParserFacade.hpp>
#include <AST/TranslationUnitNode.hpp>
#include <llvm/Support/Host.h>
#include "AST/function/FunctionDefinitionNode.hpp"
#include "AST/class/ClassDefinitionNode.hpp"
#include "types/ReferenceType.hpp"
#include "AST/variable/ClassFieldDefinitionNode.hpp"
#include "types/PointerType.hpp"
#include "types/IntegerTypes.hpp"

namespace dua
{

ModuleCompiler::ModuleCompiler(const std::string &module_name, const std::string &code) :
    context(),
    module(module_name, context),
    builder(context),
    temp_builder(context),
    name_resolver(this),
    typing_system(this)
{
    module.setTargetTriple(llvm::sys::getDefaultTargetTriple());

    ParserFacade parser(*this);

    function_scope_count.push_back(0);

    create_the_object_class();

    create_dynamic_casting_function();

    create_dua_init_function();

    // Parse
    TranslationUnitNode* ast = parser.parse(code);

    // Generate LLVM IR
    ast->eval();

    complete_dua_init_function();

    llvm::raw_string_ostream stream(result);
    module.print(stream, nullptr);
    result = stream.str();
}

void ModuleCompiler::create_dua_init_function()
{
    auto info = FunctionInfo {
        create_type<FunctionType>(create_type<VoidType>()),
        {}
    };

    name_resolver.register_function(".dua.init", std::move(info));

    auto function = module.getFunction(".dua.init.");
    llvm::BasicBlock::Create(context, "entry", function);
}

void ModuleCompiler::complete_dua_init_function()
{
    auto dua_init = module.getFunction(".dua.init.");
    auto& init_ip = dua_init->getEntryBlock();
    builder.SetInsertPoint(&init_ip);
    for (auto node : deferred_nodes)
        node->eval();

    if (dua_init->begin()->begin() == dua_init->begin()->end()) {
        // The function is empty. Delete it for clarity.
        dua_init->removeFromParent();
    } else {
        // Return
        builder.CreateRetVoid();

        // Call from main.
        auto& main_ip = module.getFunction("main")->getEntryBlock().front();
        builder.SetInsertPoint(&main_ip);
        builder.CreateCall(dua_init);
    }
}

ModuleCompiler::~ModuleCompiler()
{
    for (auto node : nodes)
        delete node;
}

void ModuleCompiler::push_scope() {
    name_resolver.push_scope();
    typing_system.push_scope();
    function_scope_count.back()++;
}

Scope<Value> ModuleCompiler::pop_scope() {
    function_scope_count.back()--;
    typing_system.pop_scope();
    return name_resolver.pop_scope();
}

void ModuleCompiler::destruct_last_scope()
{
    if (builder.GetInsertBlock()->getTerminator() != nullptr)
        return;

    name_resolver.destruct_all_variables(name_resolver.symbol_table.scopes.back());
}

void ModuleCompiler::destruct_function_scope()
{
    if (builder.GetInsertBlock()->getTerminator() != nullptr)
        return;

    auto& scopes = name_resolver.symbol_table.scopes;
    auto n = scopes.size();
    for (size_t i = 1; i <= function_scope_count.back(); i++)
        name_resolver.destruct_all_variables(scopes[n - i]);
}

void ModuleCompiler::push_scope_counter() {
    function_scope_count.push_back(0);
}

void ModuleCompiler::pop_scope_counter() {
    function_scope_count.pop_back();
}

void ModuleCompiler::create_the_object_class()
{
    name_resolver.class_id["Object"] = 0;

    std::vector<const Type*> params = { create_type<ReferenceType>(create_type<ClassType>("Object"), true) };
    auto info = FunctionInfo {
            create_type<FunctionType>(create_type<VoidType>(), params),
            {}
    };
    name_resolver.register_function("Object.destructor", std::move(info));
    auto destructor = module.getFunction("Object.destructor.Object&");
    auto bb = llvm::BasicBlock::Create(context, "entry", destructor);
    auto ip = builder.saveIP();
    builder.SetInsertPoint(bb);
    builder.CreateRetVoid();
    builder.restoreIP(ip);

    auto type = create_type<ClassType>("Object");
    typing_system.insert_type("Object", type);
    name_resolver.classes["Object"] = type;
    name_resolver.create_vtable("Object");
    name_resolver.class_fields["Object"].push_back(name_resolver.get_vtable_field("Object"));
    type->llvm_type()->setBody(name_resolver.get_vtable_type("Object")->llvm_type());
}

Value ModuleCompiler::create_string(const std::string &name, const std::string &value)
{
    auto type = create_type<PointerType>(create_type<I8Type>());
    auto it = string_pool.find(value);
    if (it != string_pool.end())
        return create_value(it->second, type);
    auto result = builder.CreateGlobalStringPtr(value, name, 0, &module);
    string_pool[value] = result;
    return create_value(result, type);
}

void ModuleCompiler::create_dynamic_casting_function()
{
    std::vector<const Type*> param_types = { create_type<PointerType>(create_type<ClassType>("Object")),
                         create_type<PointerType>(create_type<ClassType>("Object")) };
    auto info = FunctionInfo {
        create_type<FunctionType>(create_type<I8Type>(), param_types),
        {"init_vtable", "target_vtable"}
    };

    name_resolver.register_function(".is_vtable_reachable", std::move(info), true);

    auto function = module.getFunction(".is_vtable_reachable");

    // Loop while the current vtable instance pointer is not the target vtable pointer, or if
    //  you reach the root (a null pointer)
    //  This is equivalent to:
    //      auto ptr = vtable_ptr;
    //      while (ptr && ptr != target_ptr)
    //          ptr = parent_vtable(ptr);
    //      if (ptr == null) return null
    //      return pointer as target_type

    auto entry_bb = llvm::BasicBlock::Create(context, "entry", function);
    auto test_null_bb = llvm::BasicBlock::Create(context, "test_null", function);
    auto proceed_bb = llvm::BasicBlock::Create(context, "proceed", function);
    auto load_parent_bb = llvm::BasicBlock::Create(context, "load_parent", function);
    auto end_bb = llvm::BasicBlock::Create(context, "end", function);

    auto any_vtable = name_resolver.get_vtable_instance("Object");

    auto init_vtable = function->args().begin();
    auto target_vtable = function->args().begin() + 1;

    // Even though vtables of different classes are different in structure, the only part
    //  we're concerned with is the pointer of the parent vtable instance, which is always
    //  in the same position, regardless of the vtable type. For this reason, if we're only
    //  accessing the parent vtable pointer, we can treat any vtable instance as any vtable.
    auto vtable_llvm_type = init_vtable->getType();

    builder.SetInsertPoint(entry_bb);

    auto current_vtable_ptr = builder.CreateAlloca(vtable_llvm_type, nullptr, "current_vtable");
    builder.CreateStore(init_vtable, current_vtable_ptr);

    builder.CreateBr(test_null_bb);

    builder.SetInsertPoint(test_null_bb);

    llvm::Value* current_vtable = builder.CreateLoad(vtable_llvm_type->getPointerTo(), current_vtable_ptr);
    current_vtable = builder.CreatePointerCast(current_vtable, target_vtable->getType());

    auto is_null = builder.CreateICmpNE(current_vtable, llvm::Constant::getNullValue(current_vtable->getType()));
    builder.CreateCondBr(is_null, proceed_bb, end_bb);

    builder.SetInsertPoint(proceed_bb);

    auto is_target = builder.CreateICmpEQ(current_vtable, target_vtable);
    builder.CreateCondBr(is_target, end_bb, load_parent_bb);

    builder.SetInsertPoint(load_parent_bb);

    // Any vtable is usable here
    auto parent_ptr = any_vtable->get_ith_element(1, vtable_llvm_type, current_vtable);
    builder.CreateStore(parent_ptr, current_vtable_ptr);
    builder.CreateBr(test_null_bb);

    builder.SetInsertPoint(end_bb);

    auto phi = builder.CreatePHI(builder.getInt1Ty(), 2, "result");
    phi->addIncoming(builder.getInt1(1), proceed_bb);
    phi->addIncoming(builder.getInt1(0), test_null_bb);

    auto result = builder.CreateZExt(phi, builder.getInt8Ty());

    builder.CreateRet(result);
}

void ModuleCompiler::delete_dynamic_casting_function() {
    auto function = module.getFunction(".is_vtable_reachable");
    function->eraseFromParent();
}

}
