#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class NegativeExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    NegativeExpressionNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override { return type = expression->get_type(); }
};


class NotExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    NotExpressionNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override { delete type; return type = expression->get_type(); }
};


class BitwiseComplementExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    BitwiseComplementExpressionNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override { delete type; return type = expression->get_type(); }
};

}
