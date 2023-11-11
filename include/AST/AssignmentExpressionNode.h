#pragma once

#include <AST/ASTNode.h>

class AssignmentExpressionNode : public ASTNode
{
    std::string lhs;
    ASTNode* rhs;

public:

    AssignmentExpressionNode(ModuleCompiler* compiler, std::string lhs, ASTNode* rhs)
        : lhs(std::move(lhs)), rhs(rhs) { this->compiler = compiler; };
    llvm::Value* eval() override;
    ~AssignmentExpressionNode() override;
};