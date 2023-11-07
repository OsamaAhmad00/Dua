#pragma once

#include <types/TypeBase.h>

#define DEFINE_FLOAT_TYPE(WIDTH, TYPE)                \
struct F##WIDTH##Type : public FloatType              \
{                                                     \
    llvm::Constant* default_value() override {        \
        return llvm::ConstantFP::get(llvm_type(), 0); \
    }                                                 \
                                                      \
    llvm::Type* llvm_type() override {                \
        return builder->get##TYPE##Ty();              \
    }                                                 \
};

struct FloatType : public TypeBase {};

DEFINE_FLOAT_TYPE(64, Double)
DEFINE_FLOAT_TYPE(32, Float)