#pragma once

#include <AST/ASTNode.h>

class AssignmentExpressionNode : public ASTNode
{
    std::string lhs;
    ASTNode* rhs;

public:

    AssignmentExpressionNode(std::string lhs, ASTNode* rhs)
        : lhs(std::move(lhs)), rhs(rhs) {};
    llvm::Value* eval() override;
    ~AssignmentExpressionNode() override;
};