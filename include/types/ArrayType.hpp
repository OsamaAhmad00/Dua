#pragma once

#include <types/Type.hpp>
#include <llvm/IR/DerivedTypes.h>

namespace dua
{

class ArrayType : public Type
{
    size_t size;
    const Type* element_type;
    bool _is_raw;

public:

    ArrayType(ModuleCompiler* compiler, const Type* element_type, size_t size, bool is_raw = false)
        : element_type(element_type), size(size), _is_raw(is_raw) { this->compiler = compiler; }

    Value default_value() const override;

    Value zero_value() const override;

    llvm::ArrayType* llvm_type() const override;

    const Type* get_element_type() const { return element_type; }

    const Type* get_concrete_type() const override;

    std::string to_string() const override { return element_type->to_string() + "[" + std::to_string(size) + "]"; }

    std::string as_key() const override { return element_type->as_key() + "_" + (_is_raw ? "_RAW_" : "") + "Arr_" + std::to_string(size) + "_"; }

    size_t get_size() const { return size; }

    bool is_raw() const { return _is_raw; }
};

}
