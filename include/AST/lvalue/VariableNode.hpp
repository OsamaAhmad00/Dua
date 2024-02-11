#pragma once

#include "AST/ASTNode.hpp"
#include "resolution/ResolutionString.hpp"

namespace dua
{

class VariableNode : public ASTNode
{

public:

    ResolutionString* unresolved_name;
    std::vector<const Type*> template_args;
    bool is_templated;

    VariableNode(ModuleCompiler* compiler, ResolutionString* name, const Type* type = nullptr);

    VariableNode(ModuleCompiler* compiler, ResolutionString* name, std::vector<const Type*> template_args, const Type* type = nullptr);

    Value eval() override;

    const Type* get_type() override;

    [[nodiscard]] virtual std::string get_name() const;

    [[nodiscard]] virtual bool is_function() const;
};

}
