#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class PostfixAdditionExpressionNode : public ASTNode
{
    ASTNode* lhs;
    int64_t amount;

public:

    PostfixAdditionExpressionNode(ModuleCompiler* compiler, ASTNode* lhs, int64_t amount)
            : lhs(lhs), amount(amount) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}
