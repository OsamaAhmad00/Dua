#pragma once

#include <types/Type.hpp>

namespace dua
{

class ASTNode;

class TypeOfType : public Type
{
    ASTNode* node;

public:

    TypeOfType(ModuleCompiler* compiler, ASTNode* node)
            : node(node) { this->compiler = compiler; }

    const Type* get_type() const;

    Value default_value() const override;

    llvm::Type* llvm_type() const override;

    std::string to_string() const override;

    std::string as_key() const override;

    bool operator==(const Type& other) override;
};

}
