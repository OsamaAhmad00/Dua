#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class AssignmentExpressionNode : public ASTNode
{
    ASTNode* lhs;
    ASTNode* rhs;

public:

    AssignmentExpressionNode(ModuleCompiler* compiler, ASTNode* lhs, ASTNode* rhs)
        : lhs(lhs), rhs(rhs) { this->compiler = compiler; };

    Value eval() override;

    const Type* get_type() override;
};

}
