#pragma once

#include <types/TypeBase.h>

class ArrayType : public TypeBase
{
    size_t size;
    TypeBase* element_type;

public:

    ArrayType(llvm::IRBuilder<>* builder, TypeBase* element_type, size_t size)
        : element_type(element_type), size(size) { this->builder = builder; }

    llvm::Constant* default_value() override;

    llvm::ArrayType * llvm_type() override;

    ArrayType* clone() override { return new ArrayType(builder, element_type->clone(), size); }

    ~ArrayType() override { delete element_type; }
};