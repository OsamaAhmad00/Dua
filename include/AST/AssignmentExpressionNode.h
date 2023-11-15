#pragma once

#include <AST/ASTNode.h>
#include "AST/terminals/lvalue/LValueNode.h"

class AssignmentExpressionNode : public ASTNode
{
    LValueNode* lhs;
    ASTNode* rhs;

public:

    AssignmentExpressionNode(ModuleCompiler* compiler, LValueNode* lhs, ASTNode* rhs)
        : lhs(lhs), rhs(rhs) { this->compiler = compiler; };
    llvm::Value* eval() override;
    ~AssignmentExpressionNode() override;
};