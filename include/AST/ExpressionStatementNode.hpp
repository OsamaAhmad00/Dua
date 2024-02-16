#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class ExpressionStatementNode : public ASTNode
{
    ASTNode* expression;

public:

    ExpressionStatementNode(ModuleCompiler* compiler, ASTNode* expression)
            : expression(expression) { this->compiler = compiler; };

    NoneValue eval() override
    {
        compiler->push_temp_expr_scope();
        expression->eval();
        compiler->destruct_temp_expr_scope();
        return none_value();
    };
};

}
