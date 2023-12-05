#pragma once

#include <types/Type.hpp>

namespace dua
{

struct StringType : Type
{
    StringType(ModuleCompiler* compiler) { this->compiler = compiler; }

    llvm::Constant * default_value() const override {
        return llvm::Constant::getNullValue(llvm_type());
    }

    llvm::Type* llvm_type() const override {
        return compiler->get_builder()->getInt8PtrTy();
    }

    std::string to_string() const override { return "String"; }
};

}
