#pragma once

#include <types/TypeBase.h>

namespace dua
{

struct StringType : TypeBase
{
    StringType(llvm::IRBuilder<>* builder) { this->builder = builder; }

    llvm::Constant * default_value() override {
        return llvm::Constant::getNullValue(llvm_type());
    }

    llvm::Type* llvm_type() const override {
        return builder->getInt8PtrTy();
    }

    StringType* clone() override { return new StringType(builder); }
};

}
