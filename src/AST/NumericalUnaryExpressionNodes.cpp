#include <AST/unary/NumericalUnaryExpressionNodes.h>

namespace dua
{

llvm::Value* NegativeExpressionNode::eval()
{
    llvm::Value* value = expression->eval();
    if (value->getType()->isFloatTy())
        return builder().CreateFNeg(value, "neg_value");
    return builder().CreateNeg(value, "neg_value");
}

llvm::Value* NotExpressionNode::eval()
{
    llvm::Value* value = expression->eval();
    if (value->getType()->isFloatTy())
        value = builder().CreateFPToSI(value, builder().getInt8Ty(), "int_value");
    return builder().CreateNot(value, "not_value");
}

llvm::Value* BitwiseComplementExpressionNode::eval()
{
    // Applies only to integers, no floating point numbers.
    return builder().CreateXor(
        expression->eval(),
        llvm::ConstantInt::get(
            expression->eval()->getType(),
            -1
        ),
        "complement_value"
    );
};

}
