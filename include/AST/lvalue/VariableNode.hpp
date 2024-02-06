#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

class VariableNode : public ASTNode
{

public:

    std::string name;
    std::vector<const Type*> template_args;
    bool is_templated;

    VariableNode(ModuleCompiler* compiler, std::string name, const Type* type = nullptr);

    VariableNode(ModuleCompiler* compiler, std::string name, std::vector<const Type*> template_args, const Type* type = nullptr);

    Value eval() override;

    const Type* get_type() override;

    [[nodiscard]] virtual std::string get_name() const;

    [[nodiscard]] virtual bool is_function() const;
};

}
