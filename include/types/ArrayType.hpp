#pragma once

#include <types/Type.hpp>
#include <llvm/IR/DerivedTypes.h>

namespace dua
{

class ArrayType : public Type
{
    size_t size;
    Type* element_type;

public:

    ArrayType(ModuleCompiler* compiler, Type* element_type, size_t size)
        : element_type(element_type), size(size) { this->compiler = compiler; }

    llvm::Constant* default_value() override;

    llvm::ArrayType* llvm_type() const override;

    Type* get_element_type() { return element_type; }

    ArrayType* clone() override { return new ArrayType(compiler, element_type->clone(), size); }

    std::string to_string() const override { return element_type->to_string() + "[" + std::to_string(size) + "]"; }

    ~ArrayType() override { delete element_type; }
};

}
