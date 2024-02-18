#pragma once

#include <AST/ASTNode.hpp>
#include <types/ReferenceType.hpp>
#include <types/PointerType.hpp>

namespace dua
{

template <typename OpNode>
class CompoundAssignmentExpressionNode : public ASTNode
{
    ASTNode* lhs;
    ASTNode* rhs;

public:

    CompoundAssignmentExpressionNode(ModuleCompiler* compiler, ASTNode* lhs, ASTNode* rhs)
            : lhs(lhs), rhs(rhs) { this->compiler = compiler; };

    Value eval() override
    {
        auto lhs_eval = lhs->eval();
        auto rhs_eval = rhs->eval();

        auto op_name = "Compound" + OpNode::operation_name();
        auto infix = name_resolver().call_infix_operator(lhs_eval, rhs_eval, op_name);
        if (!infix.is_null())
            return infix;

        if (lhs_eval.memory_location == nullptr)
            compiler->report_error("Can't perform a compound addition to a non-lvalue expression");

        lhs_eval.type = lhs_eval.type->get_contained_type();

        auto value = OpNode::perform(
            compiler,
            lhs_eval,
            compiler->create_value(rhs_eval.get(), rhs->get_type()),
            lhs_eval.type
        );

        return AssignmentExpressionNode::perform_assignment(lhs_eval, value, compiler);
    }

    const Type* get_type() override
    {
        if (compiler->clear_type_cache)
            type = nullptr;

        if (type != nullptr) return type;

        auto op_name = "Compound" + OpNode::operation_name();
        auto infix = name_resolver().get_infix_operator_return_type(lhs->get_type(), rhs->get_type(), op_name);
        if (infix != nullptr)
            return set_type(infix);

        auto op_node = OpNode(compiler, lhs, rhs);
        auto assignment = AssignmentExpressionNode(compiler, lhs, &op_node);

        return set_type(assignment.get_type());
    }
};

}
