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

    const Type* get_element_type(const Type* type)
    {
        if (auto ref = type->as<ReferenceType>(); ref != nullptr)
            return ref->get_element_type();
        if (auto ptr = type->as<ReferenceType>(); ptr != nullptr)
            return ptr->get_element_type();
        return type;
    }

public:

    CompoundAssignmentExpressionNode(ModuleCompiler* compiler, ASTNode* lhs, ASTNode* rhs)
            : lhs(lhs), rhs(rhs) { this->compiler = compiler; };

    Value eval() override
    {
        auto lhs_eval = lhs->eval();
        if (lhs_eval.memory_location == nullptr)
            compiler->report_error("Can't perform a compound addition to a non-lvalue expression");
        lhs_eval.type = get_element_type(lhs_eval.type);
        auto rhs_value = rhs->eval();
        auto value = OpNode::perform(
            compiler,
            lhs_eval,
            compiler->create_value(rhs_value.get(), rhs->get_type()),
            lhs_eval.type
        );

        return AssignmentExpressionNode::perform_assignment(lhs_eval, value, compiler);
    }

    const Type* get_type() override
    {
        if (compiler->clear_type_cache)
            type = nullptr;

        if (type != nullptr) return type;

        auto op_node = OpNode(compiler, lhs, rhs);
        auto assignment = AssignmentExpressionNode(compiler, lhs, &op_node);

        return set_type(assignment.get_type());
    }
};

}
