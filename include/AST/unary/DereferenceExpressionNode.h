#pragma once

#include <AST/ASTNode.h>

class DereferenceExpressionNode : public ASTNode
{
    ASTNode* expression;

public:

    DereferenceExpressionNode(ModuleCompiler* compiler, ASTNode* expression)
            : expression(expression) { this->compiler = compiler; }
    // FIXME pass the value type as appropriate.
    llvm::Value * eval() override { return builder().CreateLoad(builder().getInt64Ty(), expression->eval()); };
    ~DereferenceExpressionNode() override { delete expression; };
};