#pragma once

#include <AST/ASTNode.h>

class ExpressionStatementNode : public ASTNode
{
    ASTNode* expression;

public:

    ExpressionStatementNode(ModuleCompiler* compiler, ASTNode* expression)
            : expression(expression) { this->compiler = compiler; };
    NoneValue eval() override { expression->eval(); return none_value(); };
    ~ExpressionStatementNode() override { delete expression; }
};