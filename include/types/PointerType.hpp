#pragma once

#include <types/Type.hpp>
#include <llvm/IR/DerivedTypes.h>

namespace dua
{

class PointerType : public Type
{
    const Type* element_type;

public:

    PointerType(ModuleCompiler* compiler, const Type* element_type)
            : element_type(element_type) { this->compiler = compiler; }

    Value default_value() const override;

    llvm::PointerType * llvm_type() const override;

    const Type* get_element_type() const { return element_type; }

    const Type* get_concrete_type() const override;

    std::string to_string() const override { return element_type->to_string() + "*"; }

    std::string as_key() const override { return element_type->as_key() + "ptr"; }

    bool operator==(const Type& other) const override;
};

}
