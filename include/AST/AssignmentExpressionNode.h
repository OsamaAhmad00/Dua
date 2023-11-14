#pragma once

#include <AST/ASTNode.h>
#include <AST/terminals/VariableNode.h>

class AssignmentExpressionNode : public ASTNode
{
    VariableNode* lhs;
    ASTNode* rhs;

public:

    AssignmentExpressionNode(ModuleCompiler* compiler, VariableNode* lhs, ASTNode* rhs)
        : lhs(lhs), rhs(rhs) { this->compiler = compiler; };
    llvm::Value* eval() override;
    ~AssignmentExpressionNode() override;
};