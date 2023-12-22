#pragma once

#include <types/NonConcreteType.hpp>

namespace dua
{

class IdentifierType : public DynamicType
{
    std::string name;

public:

    IdentifierType(ModuleCompiler* compiler, std::string name)
            : name(std::move(name)) { this->compiler = compiler; }

    const Type* get_concrete_type() const override;

    Value default_value() const override;

    llvm::Type* llvm_type() const override;

    std::string to_string() const override { return get_concrete_type()->to_string(); }

    std::string as_key() const override { return "Unresolved(" + name + ")"; }

    bool operator==(const Type& other) override;
};

}
