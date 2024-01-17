#pragma once

#include <types/Type.hpp>
#include <llvm/IR/DerivedTypes.h>

namespace dua
{

class ReferenceType : public Type
{
    // There are two types of references, one that is just
    //  an insertion of the same address in the symbol table,
    //  which doesn't have a corresponding memory location
    //  allocated to it to hold its address, and the other is
    //  the reference that is represented by a pointer variable,
    //  and the pointer is loaded before each access.
    // Turning an allocated reference into an unallocated reference
    //  is a form of optimization that avoids loading the address
    //  of the referenced variable on each access.
    // These two types of references are internal. To the user,
    //  both types are the same.
    bool _is_allocated;
    const Type* element_type;

public:

    ReferenceType(ModuleCompiler* compiler, const Type* element_type, bool is_allocated)
            : element_type(element_type), _is_allocated(is_allocated)
    {
        this->compiler = compiler;
        // Collapsing references
        if (auto ref = dynamic_cast<const ReferenceType*>(element_type); ref != nullptr)
            this->element_type = ref->get_element_type();
    }

    Value default_value() const override;

    llvm::Type* llvm_type() const override;

    const Type* get_element_type() const;

    const Type* get_concrete_type() const override;

    std::string to_string() const override;

    std::string as_key() const override;

    const ReferenceType* get_allocated() const;

    const ReferenceType* get_unallocated() const;

    bool operator==(const Type& other) const override;

    bool is_allocated() const;
};

}
