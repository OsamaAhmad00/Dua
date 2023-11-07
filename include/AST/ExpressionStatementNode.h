#pragma once

#include <AST/ASTNode.h>

class ExpressionStatement : public ASTNode
{
    ASTNode* expression;

public:

    ExpressionStatement(ASTNode* expression)
            : expression(expression) {};
    NoneValue eval() override { expression->eval(); return none_value(); };
    ~ExpressionStatement() override;
};