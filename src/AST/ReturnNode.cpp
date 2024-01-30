#include <AST/function/ReturnNode.hpp>
#include <types/VoidType.hpp>
#include "types/PointerType.hpp"

namespace dua
{

Value ReturnNode::eval()
{
    if (expression == nullptr)
        return compiler->create_value(builder().CreateRetVoid(), compiler->create_type<VoidType>());

    auto func = current_function()->getName().str();
    auto result = expression->eval();
    auto return_type = name_resolver().get_function_no_overloading(func).type->return_type;

    auto ptr = typing_system().cast_value(result, return_type).get();

    // Destruct the objects before returning
    compiler->destruct_function_scope();

    // The return statement has a void type. This is different from the type of
    //  the returned value, which is the type of the function call expression.
    return compiler->create_value(builder().CreateRet(ptr), get_type());
}

}
