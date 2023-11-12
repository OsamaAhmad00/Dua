#include <AST/unary/CastExpressionNode.h>

llvm::Value *CastExpressionNode::eval()
{
    llvm::Value* result = compiler->cast_value(expression->eval(), target_type);
    if (result == nullptr)
        throw std::runtime_error("Invalid cast operation");
    return result;
}

CastExpressionNode::~CastExpressionNode()
{
    delete expression;
}