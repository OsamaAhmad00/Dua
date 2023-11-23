#pragma once

#include <types/TypeBase.h>
#include <utils/ErrorReporting.h>

namespace dua
{

struct VoidType : TypeBase
{
    VoidType(llvm::IRBuilder<>* builder) { this->builder = builder; }

    llvm::Constant* default_value() override {
        report_internal_error("Void types has no value");
        return nullptr;
    }

    llvm::Type* llvm_type() const override {
        return builder->getVoidTy();
    }

    VoidType* clone() override { return new VoidType(builder); }
};

}
