#pragma once

#include "AST/variable/VariableDefinitionNode.h"

namespace dua
{

class LocalVariableDefinitionNode : public VariableDefinitionNode
{
public:

    LocalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, TypeBase* type, ASTNode* initializer)
            : VariableDefinitionNode(compiler, std::move(name), type, initializer) {}
    llvm::Value* eval() override;
};

}
