#pragma once

#include <AST/ASTNode.h>

namespace dua
{

class NegativeExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    NegativeExpressionNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }
    llvm::Value* eval() override;
    Type* compute_type() override { delete type; return type = expression->get_cached_type()->clone(); }
    ~NegativeExpressionNode() override { delete expression; }
};

class NotExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    NotExpressionNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }
    llvm::Value* eval() override;
    Type* compute_type() override { delete type; return type = expression->get_cached_type()->clone(); }
    ~NotExpressionNode() override { delete expression; }
};

class BitwiseComplementExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    BitwiseComplementExpressionNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }
    llvm::Value* eval() override;
    Type* compute_type() override { delete type; return type = expression->get_cached_type()->clone(); }
    ~BitwiseComplementExpressionNode() override { delete expression; }
};

}
