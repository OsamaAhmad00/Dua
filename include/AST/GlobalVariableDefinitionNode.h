#pragma once

#include <AST/ASTNode.h>
#include <AST/terminals/ValueNode.h>

class GlobalVariableDefinitionNode : public ASTNode
{
    std::string name;
    ValueNode* initializer;

public:

    GlobalVariableDefinitionNode(std::string name, ValueNode* initializer)
        : name(std::move(name)), initializer(initializer) {}
    llvm::GlobalVariable* eval() override;
    ~GlobalVariableDefinitionNode() override;
};