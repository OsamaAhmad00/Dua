#include <AST/AssignmentExpressionNode.hpp>
#include <utils/ErrorReporting.hpp>
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"

namespace dua
{

Value AssignmentExpressionNode::eval()
{
    auto lhs_res = lhs->eval();
    auto rhs_res = rhs->eval();

    if (auto infix = name_resolver().call_infix_operator(lhs_res, rhs_res, "Assignment"); !infix.is_null())
        return infix;

    compiler->remove_temp_expr(rhs_res.id);

    if (auto ref = lhs_res.type->as<ReferenceType>(); ref != nullptr) {
        if (ref->is_allocated()) {
            lhs_res.memory_location = lhs_res.get();
            lhs_res.set(nullptr);
        }
        lhs_res.type = ref->get_element_type();
    }

    if (lhs_res.memory_location == nullptr)
        compiler->report_error("There is no assignment operator defined for the types "
            + lhs->get_type()->to_string() + " and " + rhs->get_type()->to_string());

    if (auto ref = rhs_res.type->as<ReferenceType>(); ref != nullptr) {
        if (ref->is_allocated()) {
            rhs_res.memory_location = rhs_res.get();
            rhs_res.set(nullptr);
        }
        rhs_res.type = ref->get_element_type();
    }

    if (*lhs_res.type != *rhs_res.type) {
        Value alternative = typing_system().cast_value(rhs_res, lhs_res.type);
        if (alternative.is_null())
            compiler->report_error("There is no assignment operator defined for the types "
                                   + lhs->get_type()->to_string() + " and " + rhs->get_type()->to_string());
        rhs_res = alternative;
    }

    // If lhs is an object, call its destructor before copying
    if (auto cls = lhs_res.type->as<ClassType>(); cls != nullptr)
    {
        // Check for self-assignment first
        auto ne = builder().CreateICmpNE(lhs_res.memory_location, rhs_res.memory_location);
        auto call_destructor = create_basic_block("self_assignment_check_fail", current_function());
        auto end = create_basic_block("self_assignment_check_end", current_function());

        builder().CreateCondBr(ne, call_destructor, end);

        builder().SetInsertPoint(call_destructor);
        auto instance = compiler->create_value(lhs_res.memory_location, lhs_res.type);
        name_resolver().call_destructor(instance);
        builder().CreateBr(end);

        builder().SetInsertPoint(end);
    }

    builder().CreateStore(rhs_res.get(), lhs_res.memory_location);

    // The assignment expression a = b = c from the point of
    //  view of a (where b = c is the assignment expression)
    //  is equivalent to a = b, in which b is not teleporting.
    rhs_res.is_teleporting = false;

    return rhs_res;
}

const Type* AssignmentExpressionNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    return set_type(lhs->get_type());
}

}
