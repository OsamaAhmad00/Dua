#include <AST/unary/CastExpressionNode.h>
#include <Utils.h>

llvm::Value *CastExpressionNode::eval()
{
    llvm::Value* result = cast_value(expression->eval(), target_type, builder());
    if (result == nullptr)
        throw std::runtime_error("Invalid cast operation");
    return result;
}

CastExpressionNode::~CastExpressionNode()
{
    delete expression;
}