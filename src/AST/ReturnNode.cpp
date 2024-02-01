#include <AST/function/ReturnNode.hpp>
#include <types/VoidType.hpp>

namespace dua
{

Value ReturnNode::eval()
{
    auto func = current_function()->getName().str();
    auto return_type = name_resolver().get_function_no_overloading(func).type->return_type;

    if (expression == nullptr) {
        auto void_type = compiler->create_type<VoidType>();
        if (return_type != void_type)
            report_error("Can't return void from the function " + func + " which returns a " + return_type->to_string());
        return compiler->create_value(builder().CreateRetVoid(), void_type);
    }

    auto result = expression->eval();

    auto ptr = typing_system().cast_value(result, return_type).get();

    // Destruct the objects before returning
    compiler->destruct_function_scope();

    // The return statement has a void type. This is different from the type of
    //  the returned value, which is the type of the function call expression.
    return compiler->create_value(builder().CreateRet(ptr), get_type());
}

}
