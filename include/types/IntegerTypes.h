#pragma once

#include <types/TypeBase.h>

#define DEFINE_INTEGER_TYPE(PREFIX, WIDTH)          \
struct PREFIX##WIDTH##Type : public IntegerType     \
{                                                   \
    PREFIX##WIDTH##Type(llvm::IRBuilder<>* builder) \
        { this->builder = builder; }                \
                                                    \
    llvm::Constant* default_value() override {      \
        return builder->getInt##WIDTH(0);           \
    }                                               \
                                                    \
    llvm::Type* llvm_type() override {              \
        return builder->getInt##WIDTH##Ty();        \
    }                                               \
};

struct IntegerType : public TypeBase {};

DEFINE_INTEGER_TYPE(I, 64)
DEFINE_INTEGER_TYPE(I, 32)
DEFINE_INTEGER_TYPE(I, 16)
DEFINE_INTEGER_TYPE(I, 8 )