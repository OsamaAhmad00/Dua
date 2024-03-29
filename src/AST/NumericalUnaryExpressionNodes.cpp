#include <AST/unary/NumericalUnaryExpressionNodes.hpp>
#include "types/IntegerTypes.hpp"

namespace dua
{

Value NegativeExpressionNode::eval()
{
    auto value = expression->eval();
    llvm::Value* result;
    if (value.type->llvm_type()->isFloatingPointTy())
        result = builder().CreateFNeg(value.get(), "neg_value");
    else
        result = builder().CreateNeg(value.get(), "neg_value");
    return compiler->create_value(result, get_type());
}

Value NotExpressionNode::eval()
{
    auto value = expression->eval();
    if (value.type->llvm_type()->isFloatingPointTy())
        compiler->report_error("The not operation is not applicable to float types");
    return compiler->create_value(
            builder().CreateNot(value.cast_as_bool().get(), "not_value"),
            compiler->create_type<I8Type>())
            .cast_as(expression->get_type());
}

Value BitwiseComplementExpressionNode::eval()
{
    if (dynamic_cast<const IntegerType*>(expression->get_type()) == nullptr)
        compiler->report_error("Can't perform the bitwise complement operation on a non-integer ("
            + expression->get_type()->to_string() + ") type");
    auto result = builder().CreateXor(
        expression->eval().get(),
        llvm::ConstantInt::get(
            expression->eval().type->llvm_type(),
            -1
        ),
        "complement_value"
    );

    return compiler->create_value(result, get_type());
}

}
