#pragma once

#include <AST/ASTNode.hpp>
#include <AST/lvalue/LValueNode.hpp>

namespace dua
{

template <typename OpNode>
class CompoundAssignmentExpressionNode : public ASTNode
{
    LValueNode* lhs;
    ASTNode* rhs;

public:

    CompoundAssignmentExpressionNode(ModuleCompiler* compiler, LValueNode* lhs, ASTNode* rhs)
            : lhs(lhs), rhs(rhs) { this->compiler = compiler; };

    llvm::Value* eval() override
    {
        auto lhs_ptr = lhs->eval();
        auto lhs_value = builder().CreateLoad(lhs->get_element_type()->llvm_type(), lhs_ptr);
        auto rhs_value = rhs->eval();
        auto value = OpNode::perform(compiler, lhs_value, rhs_value, lhs->get_element_type()->llvm_type());
        builder().CreateStore(value, lhs_ptr);
        return value;
    }

    Type* compute_type() override { delete type; return type = lhs->get_cached_type()->clone(); };

    ~CompoundAssignmentExpressionNode() override { delete lhs; delete rhs; };
};

}