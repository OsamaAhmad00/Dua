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

    auto store = builder().CreateStore(rhs_res.get(), lhs_res.memory_location);
    compiler->create_value(store, lhs_res.type);

    return rhs_res;
}

const Type* AssignmentExpressionNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    return set_type(lhs->get_type());
}

}
