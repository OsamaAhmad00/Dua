#pragma once

#include <types/TypeBase.h>

class ArrayType : public TypeBase
{
    size_t size;
    TypeBase* element_type;

public:

    ArrayType(TypeBase* element_type) : element_type(element_type) {}
    llvm::Constant* default_value() override;
    llvm::ArrayType * llvm_type() override;
    ~ArrayType() override { delete element_type; }
};