#include <AST/AssignmentExpressionNode.hpp>
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"

namespace dua
{

Value AssignmentExpressionNode::eval()
{
    auto lhs_eval = lhs->eval();
    auto rhs_eval = rhs->eval();
    return perform_assignment(lhs_eval, rhs_eval, compiler);
}

Value AssignmentExpressionNode::perform_assignment(Value lhs, Value rhs, ModuleCompiler* compiler)
{
    if (auto infix = compiler->get_name_resolver().call_infix_operator(lhs, rhs, "Assignment"); !infix.is_null())
        return infix;

    compiler->remove_temp_expr(rhs.id);

    if (auto ref = lhs.type->as<ReferenceType>(); ref != nullptr) {
        if (ref->is_allocated()) {
            lhs.memory_location = lhs.get();
            lhs.set(nullptr);
        }
        lhs.type = ref->get_element_type();
    }

    if (lhs.memory_location == nullptr)
        compiler->report_error("There is no assignment operator defined for the types "
                               + lhs.type->to_string() + " and " + rhs.type->to_string());

    if (auto ref = rhs.type->as<ReferenceType>(); ref != nullptr) {
        if (ref->is_allocated()) {
            rhs.memory_location = rhs.get();
            rhs.set(nullptr);
        }
        rhs.type = ref->get_element_type();
    }

    if (*lhs.type != *rhs.type) {
        Value alternative = compiler->get_typing_system().cast_value(rhs, lhs.type);
        if (alternative.is_null())
            compiler->report_error("There is no assignment operator defined for the types "
                                   + lhs.type->to_string() + " and " + rhs.type->to_string());
        rhs = alternative;
    }

    auto& builder = *compiler->get_builder();

    builder.CreateStore(rhs.get(), lhs.memory_location);

    // If lhs is an object, call its destructor before copying
    // The destruction has to happen after the copying of the object
    if (auto cls = lhs.type->is<ClassType>(); cls != nullptr)
    {
        // Check for self-assignment first
        auto ne = builder.CreateICmpNE(lhs.memory_location, rhs.memory_location);
        auto call_destructor = compiler->create_basic_block("self_assignment_check_fail");
        auto end = compiler->create_basic_block("self_assignment_check_end");

        builder.CreateCondBr(ne, call_destructor, end);

        builder.SetInsertPoint(call_destructor);
        auto instance = compiler->create_value(lhs.memory_location, lhs.type);
        compiler->get_name_resolver().call_destructor(instance);
        builder.CreateBr(end);

        builder.SetInsertPoint(end);
    }

    // The assignment expression a = b = c from the point of
    //  view of a (where b = c is the assignment expression)
    //  is equivalent to a = b, in which b is not teleporting.
    rhs.is_teleporting = false;

    return rhs;
}

const Type* AssignmentExpressionNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    auto infix_type = name_resolver().get_infix_operator_return_type(lhs->get_type(), rhs->get_type(), "Assignment");
    if (infix_type != nullptr)
        return set_type(infix_type);

    return set_type(lhs->get_type());
}

}
