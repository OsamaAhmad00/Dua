#pragma once

#include <types/TypeBase.h>

namespace dua
{

class PointerType : public TypeBase
{
    TypeBase* element_type;

public:

    PointerType(llvm::IRBuilder<>* builder, TypeBase* element_type)
            : element_type(element_type) { this->builder = builder; }
    llvm::Constant* default_value() override;
    llvm::PointerType * llvm_type() const override;
    TypeBase* get_element_type() { return element_type; }
    PointerType* clone() override;
    ~PointerType() override { delete element_type; }
};

}
