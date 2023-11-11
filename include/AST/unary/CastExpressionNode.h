#pragma once

#include <AST/ASTNode.h>
#include <types/IntegerTypes.h>


class CastExpressionNode : public ASTNode
{
    ASTNode* expression;
    llvm::Type* target_type;

public:

    CastExpressionNode(ModuleCompiler* compiler, ASTNode* expression, llvm::Type* target_type)
        : expression(expression), target_type(target_type) { this->compiler = compiler; }
    llvm::Value * eval() override;
    ~CastExpressionNode() override;
};