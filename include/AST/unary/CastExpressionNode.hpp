#pragma once

#include <AST/ASTNode.hpp>
#include <types/Type.hpp>

namespace dua
{

class CastExpressionNode : public ASTNode
{
    ASTNode* expression;
    const Type* target_type;
    bool is_forced;

public:

    CastExpressionNode(ModuleCompiler* compiler, ASTNode* expression, const Type* target_type, bool is_forced = false)
        : expression(expression), target_type(target_type), is_forced(is_forced) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}
