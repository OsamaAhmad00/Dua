#pragma once

#include <AST/ASTNode.h>

namespace dua
{

class ExpressionStatementNode : public ASTNode
{
    ASTNode* expression;

public:

    ExpressionStatementNode(ModuleCompiler* compiler, ASTNode* expression)
            : expression(expression) { this->compiler = compiler; };
    NoneValue eval() override { expression->eval(); return none_value(); };
    TypeBase* compute_type() override { delete type; return type = expression->get_cached_type()->clone(); }
    ~ExpressionStatementNode() override { delete expression; }
};

}
