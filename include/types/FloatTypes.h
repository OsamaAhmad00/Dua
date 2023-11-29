#pragma once

#include <types/TypeBase.h>

namespace dua
{

#define DEFINE_FLOAT_TYPE(WIDTH, TYPE)                               \
struct F##WIDTH##Type : public FloatType                             \
{                                                                    \
    F##WIDTH##Type(ModuleCompiler* compiler)                         \
        { this->compiler = compiler; }                               \
                                                                     \
    llvm::Constant* default_value() override {                       \
        return llvm::ConstantFP::get(llvm_type(), 0);                \
    }                                                                \
                                                                     \
    llvm::Type* llvm_type() const override {                         \
        return compiler->get_builder()->get##TYPE##Ty();             \
    }                                                                \
                                                                     \
    F##WIDTH##Type* clone() override {                               \
        return new F##WIDTH##Type(compiler);                         \
    }                                                                \
                                                                     \
    std::string to_string() const override {                         \
        return "f"#WIDTH;                                        \
    }                                                                \
};

struct FloatType : public TypeBase {};

DEFINE_FLOAT_TYPE(64, Double)
DEFINE_FLOAT_TYPE(32, Float)

}
