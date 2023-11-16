#include <AST/unary/PostfixAdditionExpressionNode.h>
#include <AST/CompoundAssignmentExpressionNode.h>
#include <AST/binary/ArithmeticNodes.h>
#include <AST/terminals/IntegerValueNodes.h>
#include <AST/lvalue/NoDeleteLValueWrapperNode.h>

namespace dua
{

llvm::Value *PostfixAdditionExpressionNode::eval()
{
    lvalue->set_load_value(true);
    auto result = lvalue->eval();
    compiler->create_node<CompoundAssignmentExpressionNode<AdditionNode>>(
            new NoDeleteLValueWrapperNode(lvalue),
            compiler->create_node<I32ValueNode>(amount)
    )->eval();
    return result;
}

PostfixAdditionExpressionNode::~PostfixAdditionExpressionNode()
{
    delete lvalue;
}

TypeBase *PostfixAdditionExpressionNode::compute_type() {
    delete type;
    return type = lvalue->get_cached_type()->clone();
}

}
