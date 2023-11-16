#pragma once

#include <AST/ASTNode.h>
#include <types/TypeBase.h>

namespace dua
{

class CastExpressionNode : public ASTNode
{
    ASTNode* expression;
    TypeBase* target_type;

public:

    CastExpressionNode(ModuleCompiler* compiler, ASTNode* expression, TypeBase* target_type)
        : expression(expression), target_type(target_type) { this->compiler = compiler; }
    llvm::Value * eval() override;
    TypeBase* compute_type() override;
    ~CastExpressionNode() override;
};

}
