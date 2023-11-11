#pragma once

#include <AST/ASTNode.h>

class ExpressionStatement : public ASTNode
{
    ASTNode* expression;

public:

    ExpressionStatement(ModuleCompiler* compiler, ASTNode* expression)
            : expression(expression) { this->compiler = compiler; };
    NoneValue eval() override { expression->eval(); return none_value(); };
    ~ExpressionStatement() override;
};