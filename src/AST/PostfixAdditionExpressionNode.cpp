#include <AST/unary/PostfixAdditionExpressionNode.hpp>

namespace dua
{

llvm::Value *PostfixAdditionExpressionNode::eval()
{
    auto ptr = lvalue->eval();
    auto value = builder().CreateLoad(lvalue->get_element_type()->llvm_type(), ptr);
    auto rhs = builder().CreateIntCast(builder().getInt32(amount), value->getType(), true);
    auto sum = builder().CreateAdd(value, rhs);
    builder().CreateStore(sum, ptr);
    return value;
}

const Type *PostfixAdditionExpressionNode::get_type() {
    return lvalue->get_type();
}

}
