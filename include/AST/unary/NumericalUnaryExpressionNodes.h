#pragma once

#include <AST/ASTNode.h>

class NegativeExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    NegativeExpressionNode(ASTNode* expression) : expression(expression) {}
    llvm::Value* eval() override;
    ~NegativeExpressionNode() override { delete expression; }
};

class NotExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    NotExpressionNode(ASTNode* expression) : expression(expression) {}
    llvm::Value* eval() override;
    ~NotExpressionNode() override { delete expression; }
};

class BitwiseComplementExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    BitwiseComplementExpressionNode(ASTNode* expression) : expression(expression) {}
    llvm::Value* eval() override;
    ~BitwiseComplementExpressionNode() override { delete expression; }
};
