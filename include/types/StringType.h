#pragma once

#include <types/TypeBase.h>

namespace dua
{

struct StringType : TypeBase
{
    StringType(ModuleCompiler* compiler) { this->compiler = compiler; }

    llvm::Constant * default_value() override {
        return llvm::Constant::getNullValue(llvm_type());
    }

    llvm::Type* llvm_type() const override {
        return compiler->get_builder()->getInt8PtrTy();
    }

    StringType* clone() override { return new StringType(compiler); }
};

}
