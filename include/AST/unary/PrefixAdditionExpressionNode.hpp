#pragma once

#include <AST/lvalue/LValueNode.hpp>
#include <types/IntegerTypes.hpp>

namespace dua
{

class PrefixAdditionExpressionNode : public ASTNode
{
    LValueNode* lvalue;
    int64_t amount;

public:

    PrefixAdditionExpressionNode(ModuleCompiler* compiler, LValueNode* lvalue, int64_t amount)
            : lvalue(lvalue), amount(amount) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}
