#pragma once

#include <AST/ASTNode.h>

class LocalVariableDefinitionNode : public ASTNode
{
    std::string name;
    std::string type;
    ASTNode* initializer;  // Can be nullptr in case of declaration

public:
    LocalVariableDefinitionNode(std::string name, std::string type, ASTNode* initializer)
            : name(std::move(name)), type(std::move(type)), initializer(initializer) {}
    llvm::AllocaInst* eval() override;
    ~LocalVariableDefinitionNode() override;
};