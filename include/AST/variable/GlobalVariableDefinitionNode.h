#pragma once

#include "AST/variable/VariableDefinitionNode.h"

namespace dua
{

class GlobalVariableDefinitionNode : public VariableDefinitionNode
{
public:

    GlobalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, Type* type, ASTNode* initializer)
        : VariableDefinitionNode(compiler, std::move(name), type, initializer) {}
    llvm::GlobalVariable* eval() override;
};

}
