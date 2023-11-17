#include <AST/unary/PostfixAdditionExpressionNode.h>

namespace dua
{

llvm::Value *PostfixAdditionExpressionNode::eval()
{
    auto ptr = lvalue->eval();
    auto value = builder().CreateLoad(lvalue->get_element_type()->llvm_type(), ptr);
    auto sum = builder().CreateAdd(value, builder().getInt32(amount));
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
