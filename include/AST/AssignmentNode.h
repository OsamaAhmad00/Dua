#pragma once

#include <AST/ASTNode.h>

class AssignmentNode : public ASTNode
{
    std::string lhs;
    ASTNode* rhs;

public:

    AssignmentNode(std::string lhs, ASTNode* rhs)
        : lhs(std::move(lhs)), rhs(rhs) {};
    llvm::Value* eval() override;
    ~AssignmentNode() override;
};