#pragma once

#include <types/Type.hpp>
#include <llvm/IR/DerivedTypes.h>

namespace dua
{

class ArrayType : public Type
{
    size_t size;
    const Type* element_type;

public:

    ArrayType(ModuleCompiler* compiler, const Type* element_type, size_t size)
        : element_type(element_type), size(size) { this->compiler = compiler; }

    Value default_value() const override;

    llvm::ArrayType* llvm_type() const override;

    const Type* get_element_type() const { return element_type; }

    std::string to_string() const override { return element_type->to_string() + "[" + std::to_string(size) + "]"; }

    std::string as_key() const override { return element_type->as_key() + "_Arr_" + std::to_string(size) + "_"; }
};

}
