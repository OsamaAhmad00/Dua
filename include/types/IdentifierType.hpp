#pragma once

#include <types/Type.hpp>

namespace dua
{

class IdentifierType : public Type
{

public:

    std::string name;
    bool is_templated;
    std::vector<const Type*> template_args;

    IdentifierType(ModuleCompiler* compiler, std::string name, bool is_templated = false, std::vector<const Type*> template_args = {})
            : name(std::move(name)), is_templated(is_templated), template_args(std::move(template_args)) { this->compiler = compiler; }

    const Type* get_concrete_type() const override;

    bool is_resolvable_now() const override;

    Value default_value() const override;

    llvm::Type* llvm_type() const override;

    std::string to_string() const override;

    std::string as_key() const override;

    bool operator==(const Type& other) const override;
};

}
