#pragma once

#include "AST/variable/VariableDefinitionNode.hpp"

namespace dua
{

class GlobalVariableDefinitionNode : public VariableDefinitionNode
{
    Value result;

public:

    GlobalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, const Type* type,
                                 ASTNode* initializer, std::vector<ASTNode*> args = {})
        : VariableDefinitionNode(compiler, std::move(name), type, initializer, std::move(args)) {}

    Value eval() override;
};

}
