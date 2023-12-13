#pragma once

#include <AST/lvalue/LValueNode.hpp>

namespace dua
{

class PostfixAdditionExpressionNode : public ASTNode
{
    LValueNode* lvalue;
    int64_t amount;

public:

    PostfixAdditionExpressionNode(ModuleCompiler* compiler, LValueNode* lvalue, int64_t amount)
            : lvalue(lvalue), amount(amount) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}
