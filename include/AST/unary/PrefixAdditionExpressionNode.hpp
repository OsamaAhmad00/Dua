#pragma once

#include <AST/ASTNode.hpp>
#include <types/IntegerTypes.hpp>

namespace dua
{

class PrefixAdditionExpressionNode : public ASTNode
{
    ASTNode* lhs;
    int64_t amount;

public:

    PrefixAdditionExpressionNode(ModuleCompiler* compiler, ASTNode* lhs, int64_t amount)
            : lhs(lhs), amount(amount) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}
