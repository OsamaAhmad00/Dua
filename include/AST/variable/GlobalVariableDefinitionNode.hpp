#pragma once

#include "AST/variable/VariableDefinitionNode.hpp"

namespace dua
{

class GlobalVariableDefinitionNode : public VariableDefinitionNode
{
    Value result;
    bool is_extern;

public:

    GlobalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, const Type* type,
                                 ASTNode* initializer = nullptr, std::vector<ASTNode*> args = {}, bool is_extern = false)
        : VariableDefinitionNode(compiler, std::move(name), type, initializer, std::move(args)), is_extern(is_extern) {}

    Value eval() override;
};

}
