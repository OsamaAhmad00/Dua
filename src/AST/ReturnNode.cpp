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
            compiler->report_error("Can't return void from the function " + func + " which returns a " + return_type->to_string());
        return compiler->create_value(builder().CreateRetVoid(), void_type);
    }

    auto return_value = expression->eval().cast_as(return_type);

    auto result_ptr = builder().CreateAlloca(return_type->llvm_type(), nullptr, "return_value");
    auto result = compiler->create_value(result_ptr, return_type);

    // This is to force the loading of the returned value before
    //  returning it, to avoid returning stale versions.
    if (return_value.memory_location != nullptr)
        return_value.set(nullptr);

    name_resolver().copy_construct(result, return_value);

    // Moving the address to the memory_location field
    result.memory_location = result.get();
    result.set(nullptr);

    // Destruct the objects before returning
    compiler->destruct_function_scope();

    // The return statement has a void type. This is different from the type of
    //  the returned value, which is the type of the function call expression.
    return compiler->create_value(builder().CreateRet(result.get()), get_type());
}

}
