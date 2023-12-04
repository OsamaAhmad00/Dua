#pragma once

#include "AST/variable/VariableDefinitionNode.hpp"

namespace dua
{

class GlobalVariableDefinitionNode : public VariableDefinitionNode
{
    llvm::GlobalVariable* result = nullptr;

public:

    GlobalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, Type* type,
                                 ASTNode* initializer, std::vector<ASTNode*> args = {})
        : VariableDefinitionNode(compiler, std::move(name), type, initializer, std::move(args)) {}

    llvm::GlobalVariable* eval() override;
};

}
