#pragma once

#include <types/TypeBase.h>

struct StringType : TypeBase
{
    llvm::Constant * default_value() override {
        return llvm::Constant::getNullValue(llvm_type());
    }

    llvm::Type* llvm_type() override {
        return builder->getInt8PtrTy();
    }
};