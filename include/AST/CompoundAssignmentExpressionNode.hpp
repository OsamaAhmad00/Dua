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
        auto lhs_type = lhs->get_element_type();
        auto lhs_value = builder().CreateLoad(lhs_type->llvm_type(), lhs_ptr);
        auto rhs_value = rhs->eval();
        auto value = OpNode::perform(
            compiler,
            compiler->create_value(lhs_value, lhs_type),
            compiler->create_value(rhs_value, rhs->get_type()),
            lhs->get_element_type()
        );
        builder().CreateStore(value, lhs_ptr);
        return value;
    }

    const Type* get_type() override { return type = lhs->get_element_type(); };
};

}
