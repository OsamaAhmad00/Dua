#include <AST/unary/PostfixAdditionExpressionNode.hpp>
#include "types/IntegerTypes.hpp"
#include "types/ReferenceType.hpp"

namespace dua
{

static const Type* get_element_type(const Type* type)
{
    if (auto ref = type->as<ReferenceType>(); ref != nullptr)
        return ref->get_element_type();
    if (auto ptr = type->as<ReferenceType>(); ptr != nullptr)
        return ptr->get_element_type();
    return type;
}

Value PostfixAdditionExpressionNode::eval()
{
    auto type = get_element_type(lhs->get_type());
    if (type->as<IntegerType>() == nullptr)
        compiler->report_error("Can't perform postfix increment/decrement on non-integer (" + type->to_string() + ") types.");
    auto lhs_eval = lhs->eval();
    if (lhs_eval.memory_location == nullptr)
        report_error("Can't perform a prefix increment/decrement to a non-lvalue expression");
    auto value = builder().CreateLoad(type->llvm_type(), lhs_eval.memory_location);
    auto rhs = builder().CreateIntCast(builder().getInt32(amount), value->getType(), true);
    auto sum = builder().CreateAdd(value, rhs);
    builder().CreateStore(sum, lhs_eval.memory_location);
    return compiler->create_value(value, get_type());
}

const Type* PostfixAdditionExpressionNode::get_type()
{
    if (compiler->clear_type_cache)
        type = nullptr;

    if (type != nullptr) return type;

    return set_type(get_element_type(lhs->get_type()));
}


}
