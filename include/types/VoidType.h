#pragma once

#include <types/TypeBase.h>

struct VoidType : TypeBase
{
    VoidType(llvm::IRBuilder<>* builder) { this->builder = builder; }

    llvm::Constant* default_value() override {
        throw std::runtime_error("Void types has no value");
    }

    llvm::Type* llvm_type() override {
        return builder->getVoidTy();
    }

    VoidType* clone() override { return new VoidType(builder); }
};