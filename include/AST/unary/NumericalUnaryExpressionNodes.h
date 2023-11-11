#pragma once

#include <AST/ASTNode.h>

class NegativeExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    NegativeExpressionNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }
    llvm::Value* eval() override;
    ~NegativeExpressionNode() override { delete expression; }
};

class NotExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    NotExpressionNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }
    llvm::Value* eval() override;
    ~NotExpressionNode() override { delete expression; }
};

class BitwiseComplementExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    BitwiseComplementExpressionNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }
    llvm::Value* eval() override;
    ~BitwiseComplementExpressionNode() override { delete expression; }
};
