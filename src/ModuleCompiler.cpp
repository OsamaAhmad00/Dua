#include <ModuleCompiler.hpp>
#include <parsing/ParserFacade.hpp>
#include <AST/TranslationUnitNode.hpp>
#include <llvm/Support/Host.h>
#include <utils/ErrorReporting.hpp>
#include <llvm/IR/Verifier.h>
#include <types/PointerType.hpp>

namespace dua
{

ModuleCompiler::ModuleCompiler(const std::string &module_name, const std::string &code) :
    context(),
    module(module_name, context),
    builder(context),
    temp_builder(context),
    name_resolver(this)
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

    delete ast;
}

llvm::Value* ModuleCompiler::cast_value(llvm::Value* value, llvm::Type* target_type, bool panic_on_failure)
{
    llvm::Type* source_type = value->getType();

    // If the types are equal, return the value as it is
    if (source_type == target_type) {
        return value;
    }

    llvm::DataLayout dl(&module);
    unsigned int source_width = dl.getTypeAllocSize(source_type);
    unsigned int target_width = dl.getTypeAllocSize(target_type);

    // If the types are both integer types, use the Trunc or ZExt or SExt instructions
    if (source_type->isIntegerTy() && target_type->isIntegerTy())
    {
        if (source_width >= target_width) {
            // This needs to be >=, not just > because of the case of
            //  converting between an i8 and i1, which both give a size
            //  of 1 byte.
            // Truncate the value to fit the smaller type
            return builder.CreateTrunc(value, target_type);
        } else if (source_width < target_width) {
            // Extend the value to fit the larger type
            return builder.CreateSExt(value, target_type);
        }
    }

    if (source_type->isPointerTy() && target_type->isPointerTy())
        return builder.CreateBitCast(value, target_type);

    if (source_type->isIntegerTy() && target_type->isPointerTy())
        return builder.CreateIntToPtr(value, target_type);

    if (source_type->isFloatingPointTy() && target_type->isFloatingPointTy()) {
        if (source_width > target_width) {
            // Truncate the value to fit the smaller type
            return builder.CreateFPTrunc(value, target_type);
        } else if (source_width < target_width) {
            // Extend the value to fit the larger type
            return builder.CreateFPExt(value, target_type);
        } else {
            // The types have the same bit width, no need to cast
            return value;
        }
    }

    if (source_type->isIntegerTy() && target_type->isFloatingPointTy())
        return builder.CreateSIToFP(value, target_type);

    if (source_type->isFloatingPointTy() && target_type->isIntegerTy())
        return builder.CreateFPToSI(value, target_type);

    source_type->print(llvm::outs());
    llvm::outs() << '\n';
    target_type->print(llvm::outs());
    llvm::outs() << '\n';

    if (panic_on_failure)
        report_error("Casting couldn't be done");

    return nullptr;
}

Type* ModuleCompiler::get_winning_type(Type* lhs, Type* rhs)
{
    auto l = lhs->llvm_type();
    auto r = rhs->llvm_type();

    if (l == r) {
        return lhs;
    }

    llvm::DataLayout dl(&module);
    unsigned int l_width = dl.getTypeAllocSize(l);
    unsigned int r_width = dl.getTypeAllocSize(r);

    if (l->isIntegerTy() && r->isIntegerTy())
        return (l_width >= r_width) ? lhs : rhs;

    if (l->isPointerTy() && r->isIntegerTy())
        return lhs;

    if (l->isIntegerTy() && r->isPointerTy())
        return rhs;

    if (l->isFloatingPointTy() && r->isFloatingPointTy())
        return (l_width >= r_width) ? lhs : rhs;

    if (l->isIntegerTy() && r->isFloatingPointTy())
        return rhs;

    if (l->isFloatingPointTy() && r->isIntegerTy())
        return rhs;

    report_internal_error("Type mismatch");
    return nullptr;
}

llvm::Value *ModuleCompiler::cast_as_bool(llvm::Value *value, bool panic_on_failure) {
    value = cast_value(value, builder.getInt64Ty(), panic_on_failure);
    return builder.CreateICmpNE(value, builder.getInt64(0));
}

void ModuleCompiler::create_dua_init_function()
{
    auto info = FunctionInfo {
        FunctionType { this, create_type<VoidType>() },
        {}
    };

    name_resolver.register_function(".dua.init", std::move(info));

    auto function = module.getFunction(".dua.init");
    llvm::BasicBlock::Create(context, "entry", function);
}

void ModuleCompiler::complete_dua_init_function()
{
    auto& init_ip = module.getFunction(".dua.init")->getEntryBlock();
    builder.SetInsertPoint(&init_ip);
    for (auto node : deferred_nodes)
        node->eval();

    auto dua_init = module.getFunction(".dua.init");
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

}
