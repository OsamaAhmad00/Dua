#include <Utils.h>

llvm::Value* cast_value(llvm::Value* value, llvm::Type* target_type, llvm::IRBuilder<>& builder)
{
    llvm::Type* source_type = value->getType();

    // If the types are equal, return the value as it is
    if (source_type == target_type) {
        return value;
    }

    unsigned int source_width = source_type->getIntegerBitWidth();
    unsigned int target_width = target_type->getIntegerBitWidth();

    // If the types are both integer types, use the Trunc or ZExt or SExt instructions
    if (source_type->isIntegerTy() && target_type->isIntegerTy())
    {
        if (source_width > target_width) {
            // Truncate the value to fit the smaller type
            return builder.CreateTrunc(value, target_type);
        } else if (source_width < target_width) {
            // Extend the value to fit the larger type
            return builder.CreateSExt(value, target_type);
        } else {
            // The types have the same bit width, no need to cast
            return value;
        }
    }

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
