#pragma once

#include <AST/ASTNode.h>

class ReturnNode : public ASTNode
{
    ASTNode* expression;

public:

    ReturnNode(ASTNode* expression) : expression(expression) {}
    llvm::ReturnInst* eval() override;
    ~ReturnNode() override;
};