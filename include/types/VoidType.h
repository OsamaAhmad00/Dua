#pragma once

#include <types/Type.h>
#include <utils/ErrorReporting.h>

namespace dua
{

struct VoidType : Type
{
    VoidType(ModuleCompiler* compiler) { this->compiler = compiler; }

    llvm::Constant* default_value() override {
        report_internal_error("Void types has no value");
        return nullptr;
    }

    llvm::Type* llvm_type() const override {
        return compiler->get_builder()->getVoidTy();
    }

    std::string to_string() const override { return "Void"; }

    VoidType* clone() override { return new VoidType(compiler); }
};

}
