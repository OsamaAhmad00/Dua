#include <ModuleCompiler.h>
#include "parsing/Parser.h"
#include <AST/TranslationUnitNode.h>
#include <sstream>


ModuleCompiler::ModuleCompiler(const std::string &module_name, const std::string &code) :
    context(),
    module(module_name, context),
    builder(context),
    temp_builder(context),
    current_function(nullptr)
{
    Parser parser(*this);

    // Parse
    TranslationUnitNode* ast = parser.parse(code);

    // Generate LLVM IR
    ast->eval();

    // Print code to stdout
    module.print(llvm::outs(), nullptr);

    // Save code to file
    std::error_code error;
    llvm::raw_fd_ostream out(module_name + ".ll", error);
    module.print(out, nullptr);

    llvm::raw_string_ostream stream(result);
    module.print(stream, nullptr);
    result = stream.str();

    delete ast;
}

llvm::Value* ModuleCompiler::cast_value(llvm::Value* value, llvm::Type* target_type)
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

    if (source_type->isPointerTy() && target_type->isIntegerTy())
        return builder.CreatePtrToInt(value, target_type);

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

    // If none of the above cases apply, use the BitCast instruction
    // This will reinterpret the bits of the value as the target type, without changing them
    // This may not preserve the semantics of the value, and should be used with caution
    if (source_width == target_width)
        return builder.CreateBitCast(value, target_type);

    return nullptr;
}

TypeBase* ModuleCompiler::get_winning_type(TypeBase* lhs, TypeBase* rhs)
{

}