#pragma once

#include "AST/variable/VariableDefinitionNode.hpp"

namespace dua
{

class GlobalVariableDefinitionNode : public VariableDefinitionNode
{
    Value result;
    bool is_extern;
    bool is_static;

public:

    GlobalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, const Type* type,
                                 ASTNode* initializer = nullptr, std::vector<ASTNode*> args = {}, bool is_extern = false, bool is_static = false)
        : VariableDefinitionNode(compiler, std::move(name), type, initializer, std::move(args)), is_extern(is_extern), is_static(is_static) {}

    Value eval() override;
};

}
