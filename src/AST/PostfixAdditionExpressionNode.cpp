#include <AST/unary/PostfixAdditionExpressionNode.hpp>
#include "types/IntegerTypes.hpp"

namespace dua
{

Value PostfixAdditionExpressionNode::eval()
{
    if (dynamic_cast<const IntegerType*>(lvalue->get_element_type()) == nullptr)
        report_error("Can't perform postfix increment/decrement on non-integer (" +
            lvalue->get_element_type()->to_string() + ") types.");
    auto ptr = lvalue->eval();
    auto value = builder().CreateLoad(lvalue->get_element_type()->llvm_type(), ptr.get());
    auto rhs = builder().CreateIntCast(builder().getInt32(amount), value->getType(), true);
    auto sum = builder().CreateAdd(value, rhs);
    builder().CreateStore(sum, ptr.get());
    return compiler->create_value(value, get_type());
}

const Type *PostfixAdditionExpressionNode::get_type() {
    return lvalue->get_element_type();
}

}
