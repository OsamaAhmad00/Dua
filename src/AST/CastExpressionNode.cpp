#include <AST/unary/CastExpressionNode.hpp>
#include <utils/ErrorReporting.hpp>

namespace dua
{

llvm::Value *CastExpressionNode::eval()
{
    if (is_forced)
        return typing_system().forced_cast_value(expression->eval(), target_type);

    llvm::Value* result = typing_system().cast_value(
            compiler->create_value(expression->eval(), expression->get_type()), target_type);
    if (result == nullptr)
        report_error("Invalid cast operation");
    return result;
}

const Type *CastExpressionNode::get_type() {
    return target_type;
}

}
