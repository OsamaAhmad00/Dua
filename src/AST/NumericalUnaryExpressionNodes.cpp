#include <AST/unary/NumericalUnaryExpressionNodes.hpp>
#include "types/IntegerTypes.hpp"

namespace dua
{

llvm::Value* NegativeExpressionNode::eval()
{
    llvm::Value* value = expression->eval();
    if (value->getType()->isFloatingPointTy())
        return builder().CreateFNeg(value, "neg_value");
    return builder().CreateNeg(value, "neg_value");
}

llvm::Value* NotExpressionNode::eval()
{
    auto value = compiler->create_value(expression->eval(), expression->get_type());
    if (value.type->llvm_type()->isFloatingPointTy())
        report_error("The not operation is not applicable to float types");
    return compiler->create_value(
            builder().CreateNot(value.cast_as_bool(), "not_value"),
            compiler->create_type<I8Type>())
            .cast_as(expression->get_type());
}

llvm::Value* BitwiseComplementExpressionNode::eval()
{
    if (dynamic_cast<const IntegerType*>(expression->get_type()) == nullptr)
        report_error("Can't perform the bitwise complement operation on a non-integer ("
            + expression->get_type()->to_string() + ") type");
    return builder().CreateXor(
        expression->eval(),
        llvm::ConstantInt::get(
            expression->eval()->getType(),
            -1
        ),
        "complement_value"
    );
}

}
