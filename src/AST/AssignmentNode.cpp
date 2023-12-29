#include <AST/AssignmentExpressionNode.hpp>
#include <utils/ErrorReporting.hpp>
#include "types/PointerType.hpp"

namespace dua
{

Value AssignmentExpressionNode::eval()
{
    auto lhs_res = lhs->eval();
    auto rhs_res = rhs->eval();

    if (auto infix = name_resolver().call_infix_operator(lhs_res, rhs_res, "Assignment"); !infix.is_null())
        return infix;

    if (lhs_res.type->as<ReferenceType>() != nullptr)
        lhs_res.memory_location = lhs_res.get();

    if (lhs_res.memory_location == nullptr)
        report_error("There is no assignment operator defined for the types "
            + lhs_res.type->to_string() + " and " + rhs_res.type->to_string());

    if (*lhs_res.type != *rhs_res.type) {
        Value alternative = typing_system().cast_value(rhs_res, lhs_res.type);
        if (alternative.is_null())
            report_error("Invalid assignment operation");
        rhs_res = alternative;
    }

    compiler->create_value(builder().CreateStore(rhs_res.get(), lhs_res.memory_location), get_type());

    return rhs_res;
}

const Type* AssignmentExpressionNode::get_type() {
    return set_type(lhs->get_type());
}

}
