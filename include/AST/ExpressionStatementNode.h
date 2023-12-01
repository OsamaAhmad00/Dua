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
    Type* compute_type() override { if (type == nullptr) type = compiler->create_type<VoidType>(); return type; }
    ~ExpressionStatementNode() override { delete expression; }
};

}
