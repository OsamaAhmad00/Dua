#pragma once

#include <types/TypeBase.h>

class PointerType : public TypeBase
{
    TypeBase* element_type;

public:

    PointerType(llvm::IRBuilder<>* builder, TypeBase* element_type)
            : element_type(element_type) { this->builder = builder; }
    llvm::Constant* default_value() override;
    llvm::PointerType * llvm_type() override;
    ~PointerType() override { delete element_type; }
};