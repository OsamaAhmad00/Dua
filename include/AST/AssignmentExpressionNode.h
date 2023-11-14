#pragma once

#include <AST/ASTNode.h>

class AssignmentExpressionNode : public ASTNode
{
    ASTNode* lhs;
    ASTNode* rhs;

public:

    AssignmentExpressionNode(ModuleCompiler* compiler, ASTNode* lhs, ASTNode* rhs)
        : lhs(lhs), rhs(rhs) { this->compiler = compiler; };
    llvm::Value* eval() override;
    ~AssignmentExpressionNode() override;
};