#include <AST/function/ReturnNode.hpp>
#include <types/VoidType.hpp>

namespace dua
{

Value ReturnNode::eval()
{
    if (expression == nullptr)
        return compiler->create_value(builder().CreateRetVoid(), compiler->create_type<VoidType>());
    auto result = expression->eval();
    auto return_type = name_resolver().get_function_no_overloading(
            current_function()->getName().str()).type->return_type;
    auto casted = typing_system().cast_value(result, return_type);
    return compiler->create_value(builder().CreateRet(casted.ptr), get_type());
}

}
