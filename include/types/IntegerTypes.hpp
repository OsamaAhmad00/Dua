#pragma once

#include <types/Type.hpp>

namespace dua
{

#define DEFINE_INTEGER_TYPE(PREFIX, WIDTH, SIZE_ORDER)         \
struct PREFIX##WIDTH##Type : public IntegerType                \
{                                                              \
    PREFIX##WIDTH##Type(ModuleCompiler* compiler)              \
        { this->compiler = compiler; }                         \
                                                               \
    Value default_value() const override {                     \
        return compiler->create_value(                         \
            compiler->get_builder()->getInt##WIDTH(0),         \
            this                                               \
        );                                                     \
    }                                                          \
                                                               \
    llvm::Type* llvm_type() const override {                   \
        return compiler->get_builder()->getInt##WIDTH##Ty();   \
    }                                                          \
                                                               \
    std::string to_string() const override {                   \
        return "i"#WIDTH;                                      \
    }                                                          \
                                                               \
    std::string as_key() const override {                      \
        return #PREFIX#WIDTH;                                  \
    }                                                          \
                                                               \
    int size_order() const override { return SIZE_ORDER; }     \
};

struct IntegerType : public Type
{
    virtual int size_order() const = 0;
};

DEFINE_INTEGER_TYPE(I, 64, 4)
DEFINE_INTEGER_TYPE(I, 32, 3)
DEFINE_INTEGER_TYPE(I, 16, 2)
DEFINE_INTEGER_TYPE(I, 8 , 1)

}
