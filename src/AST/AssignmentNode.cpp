#include <AST/AssignmentExpressionNode.hpp>
#include <AST/lvalue/LoadedLValueNode.hpp>
#include <utils/ErrorReporting.hpp>
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"
#include "AST/IndexingNode.hpp"

namespace dua
{

Value AssignmentExpressionNode::eval()
{
    auto lhs_res = lhs->eval();
    auto rhs_res = rhs->eval();

    if (auto infix = name_resolver().call_infix_operator(lhs_res, rhs_res, "Assignment"); !infix.is_null())
        return infix;

    // Here, we're assigning an expression to an expression.
    // Since there is no infix operator defined to these two
    //  types, the lhs must be a LoadedLValueNode, or an
    //  IndexingNode, and we should assign to the memory_location of it.
    auto loaded = lhs->as<LoadedLValueNode>();
    auto indexing = lhs->as<IndexingNode>();
    assert(loaded || indexing);

    if (indexing != nullptr && lhs_res.memory_location == nullptr)
    {
        // If the lhs is an IndexingNode, and its value is not loaded,
        //  it should be turned into a loaded value first (put the result
        //  into the memory location, and turn the type into its element type)
        const Type* element_type = nullptr;
        if (auto ref = lhs_res.type->as<ReferenceType>(); ref != nullptr)
            element_type = ref->get_element_type();
        else if (auto ptr = lhs_res.type->as<ReferenceType>(); ptr != nullptr)
            element_type = ptr->get_element_type();
        else
            report_error("There is no assignment operator defined for the types "
                         + lhs_res.type->to_string() + " and " + rhs_res.type->to_string());

        lhs_res.memory_location = lhs_res.get();
        lhs_res.set(nullptr);
        lhs_res.type = element_type;
    }

    if (auto ref = lhs_res.type->as<ReferenceType>(); ref != nullptr) {
        // When assigning to an allocated type, you need to load the pointer first
        if (ref->is_allocated()) {
            lhs_res.memory_location = lhs_res.get();
            lhs_res.type = ref->get_unallocated();
        }
    }

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

const Type* AssignmentExpressionNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    return set_type(lhs->get_type());
}

}
