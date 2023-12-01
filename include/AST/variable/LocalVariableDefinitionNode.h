#pragma once

#include "AST/variable/VariableDefinitionNode.h"

namespace dua
{

class LocalVariableDefinitionNode : public VariableDefinitionNode
{
public:

    LocalVariableDefinitionNode(ModuleCompiler* compiler, std::string name,
                                Type* type, ASTNode* initializer, std::vector<ASTNode*> args = {})
            : VariableDefinitionNode(compiler, std::move(name), type, initializer, std::move(args)) {}
    llvm::Value* eval() override;
};

}
