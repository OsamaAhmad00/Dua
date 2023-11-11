#pragma once

#include <types/TypeBase.h>

struct StringType : TypeBase
{
    StringType(llvm::IRBuilder<>* builder) { this->builder = builder; }

    llvm::Constant * default_value() override {
        return llvm::Constant::getNullValue(llvm_type());
    }

    llvm::Type* llvm_type() override {
        return builder->getInt8PtrTy();
    }
};