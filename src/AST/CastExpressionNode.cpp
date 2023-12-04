#include <AST/unary/CastExpressionNode.hpp>
#include <utils/ErrorReporting.hpp>

namespace dua
{

llvm::Value *CastExpressionNode::eval()
{
    llvm::Value* result = compiler->cast_value(expression->eval(), target_type->llvm_type());
    if (result == nullptr)
        report_error("Invalid cast operation");
    return result;
}

CastExpressionNode::~CastExpressionNode()
{
    delete target_type;
    delete expression;
}

Type *CastExpressionNode::compute_type() {
    delete type;
    return type = target_type->clone();
}

}
