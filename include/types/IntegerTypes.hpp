#pragma once

#include <types/Type.hpp>

namespace dua
{

#define DEFINE_INTEGER_TYPE(PREFIX, WIDTH)                     \
struct PREFIX##WIDTH##Type : public IntegerType                \
{                                                              \
    PREFIX##WIDTH##Type(ModuleCompiler* compiler)              \
        { this->compiler = compiler; }                         \
                                                               \
    llvm::Constant* default_value() const override {           \
        return compiler->get_builder()->getInt##WIDTH(0);      \
    }                                                          \
                                                               \
    llvm::Type* llvm_type() const override {                   \
        return compiler->get_builder()->getInt##WIDTH##Ty();   \
    }                                                          \
                                                               \
    std::string to_string() const override {                   \
        return "i"#WIDTH;                                      \
    }                                                          \
};

struct IntegerType : public Type {};

DEFINE_INTEGER_TYPE(I, 64)
DEFINE_INTEGER_TYPE(I, 32)
DEFINE_INTEGER_TYPE(I, 16)
DEFINE_INTEGER_TYPE(I, 8 )

}
