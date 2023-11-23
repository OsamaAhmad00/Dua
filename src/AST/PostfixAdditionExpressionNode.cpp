#include <AST/unary/PostfixAdditionExpressionNode.h>

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

TypeBase *PostfixAdditionExpressionNode::compute_type() {
    delete type;
    return type = lvalue->get_cached_type()->clone();
}

PostfixAdditionExpressionNode::~PostfixAdditionExpressionNode()
{
    delete lvalue;
}

}
