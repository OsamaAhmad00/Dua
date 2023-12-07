#pragma once

#include <types/Type.hpp>

namespace dua
{

#define DEFINE_FLOAT_TYPE(WIDTH, TYPE)                               \
struct F##WIDTH##Type : public FloatType                             \
{                                                                    \
    F##WIDTH##Type(ModuleCompiler* compiler)                         \
        { this->compiler = compiler; }                               \
                                                                     \
    llvm::Constant* default_value() const override {                 \
        return llvm::ConstantFP::get(llvm_type(), 0);                \
    }                                                                \
                                                                     \
    llvm::Type* llvm_type() const override {                         \
        return compiler->get_builder()->get##TYPE##Ty();             \
    }                                                                \
                                                                     \
    std::string to_string() const override {                         \
        return "f"#WIDTH;                                            \
    }                                                                \
                                                                     \
    std::string as_key() const override {                            \
        return "F"#WIDTH;                                            \
    }                                                                \
};

struct FloatType : public Type {};

DEFINE_FLOAT_TYPE(64, Double)
DEFINE_FLOAT_TYPE(32, Float)

}
