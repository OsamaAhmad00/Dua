#pragma once

#include <AST/ASTNode.hpp>
#include "AST/lvalue/LValueNode.hpp"

namespace dua
{

class AssignmentExpressionNode : public ASTNode
{
    LValueNode* lhs;
    ASTNode* rhs;

public:

    AssignmentExpressionNode(ModuleCompiler* compiler, LValueNode* lhs, ASTNode* rhs)
        : lhs(lhs), rhs(rhs) { this->compiler = compiler; };

    llvm::Value* eval() override;

    const Type* get_type() override;
};

}
