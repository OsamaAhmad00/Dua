#include <ModuleCompiler.hpp>
#include <parsing/ParserFacade.hpp>
#include <AST/TranslationUnitNode.hpp>
#include <llvm/Support/Host.h>

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

    // Parse
    TranslationUnitNode* ast = parser.parse(code);

    create_dua_init_function();

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

}
