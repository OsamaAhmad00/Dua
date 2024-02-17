#include <AST/unary/CastExpressionNode.hpp>

namespace dua
{

Value CastExpressionNode::eval()
{
    if (is_forced)
        return typing_system().forced_cast_value(expression->eval(), target_type);

    Value result = typing_system().cast_value(expression->eval(), target_type);
    if (result.is_null())
        compiler->report_error("Invalid cast operation");
    return result;
}

const Type *CastExpressionNode::get_type() {
    return target_type;
}

}
