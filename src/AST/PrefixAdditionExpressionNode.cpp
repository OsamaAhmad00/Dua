#include <AST/unary/PrefixAdditionExpressionNode.hpp>
#include "types/IntegerTypes.hpp"

namespace dua
{

Value PrefixAdditionExpressionNode::eval()
{
    if (lvalue->get_element_type()->as<IntegerType>() == nullptr)
        report_error("Can't perform prefix increment/decrement on non-integer (" +
            lvalue->get_element_type()->to_string() + ") types.");
    auto ptr = lvalue->eval();
    auto value = builder().CreateLoad(lvalue->get_element_type()->llvm_type(), ptr.get());
    auto rhs = builder().CreateIntCast(builder().getInt32(amount), value->getType(), true);
    auto sum = builder().CreateAdd(value, rhs);
    builder().CreateStore(sum, ptr.get());
    return compiler->create_value(sum, get_type());
}

const Type *PrefixAdditionExpressionNode::get_type() {
    return lvalue->get_element_type();
}

}
