#pragma once

#include <types/Type.hpp>

namespace dua
{

struct VoidType : Type
{
    VoidType(ModuleCompiler* compiler) { this->compiler = compiler; }

    Value default_value() const override {
        compiler->report_internal_error("Void types have no value");
        return {};
    }

    llvm::Type* llvm_type() const override {
        return compiler->get_builder()->getVoidTy();
    }

    std::string to_string() const override { return "Void"; }

    std::string as_key() const override { return "Void"; }
};

}
