#pragma once

#include <types/Type.hpp>
#include <llvm/IR/DerivedTypes.h>

namespace dua
{

class ReferenceType : public Type
{
    const Type* element_type;

public:

    ReferenceType(ModuleCompiler* compiler, const Type* element_type)
            : element_type(element_type)
    {
        this->compiler = compiler;
        // Collapsing references
        if (auto ref = dynamic_cast<const ReferenceType*>(element_type); ref != nullptr)
            this->element_type = ref->get_element_type();
    }

    Value default_value() const override;

    llvm::Type* llvm_type() const override { return element_type->llvm_type(); }

    const Type* get_element_type() const { return element_type; }

    const Type* get_concrete_type() const override;

    std::string to_string() const override { return element_type->to_string() + "&"; }

    std::string as_key() const override { return element_type->as_key() + "ref"; }

    bool operator==(const Type& other) const override { return *element_type == other; }
};

}
