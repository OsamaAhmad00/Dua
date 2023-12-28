#pragma once

#include <types/ReferenceType.hpp>

namespace dua
{

class NoRefType : public Type
{
    const Type* element_type;

public:

    NoRefType(ModuleCompiler* compiler, const Type* element_type)
            : element_type(element_type)
    {
        this->compiler = compiler;
    }

    Value default_value() const override {
        report_internal_error("Reference types must be initialized");
        return {};
    }

    const Type* get_concrete_type() const override {
        auto type = element_type->get_concrete_type();
        if (auto r = type->as<ReferenceType>(); r != nullptr)
            return r->get_element_type();
        return type;
    }

    llvm::Type* llvm_type() const override { return get_concrete_type()->llvm_type(); }

    const Type* get_element_type() const { return element_type; }

    std::string to_string() const override { return get_concrete_type()->to_string(); }

    std::string as_key() const override { return element_type->as_key() + "noref"; }

    bool operator==(const Type& other) const override { return *get_concrete_type() == other; }
};

}
