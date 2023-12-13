#pragma once

#include "AST/variable/VariableDefinitionNode.hpp"

namespace dua
{

class LocalVariableDefinitionNode : public VariableDefinitionNode
{
public:

    LocalVariableDefinitionNode(ModuleCompiler* compiler, std::string name,
                                const Type* type, ASTNode* initializer, std::vector<ASTNode*> args = {})
            : VariableDefinitionNode(compiler, std::move(name), type, initializer, std::move(args)) {}

    Value eval() override;
};

}
