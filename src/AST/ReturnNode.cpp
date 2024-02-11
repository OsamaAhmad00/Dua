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

    auto result = expression->eval().cast_as(return_type);

    auto class_type = return_type->as<ClassType>();
    bool is_lvalue_object = class_type != nullptr && result.memory_location != nullptr;

    Value vtable_ptr;
    llvm::Value* old_vtable;
    if (is_lvalue_object)
    {
        // The get_field method expects a pointer and not the object itself
        auto instance = compiler->create_value(result.memory_location, result.type);
        vtable_ptr = class_type->get_field(instance, ".vtable_ptr");
        old_vtable = compiler->swap_vtables(vtable_ptr.get());
    }

    // Destruct the objects before returning
    compiler->destruct_function_scope();

    // Restore the old vtable
    if (is_lvalue_object)
        compiler->restore_vtable(vtable_ptr.get(), old_vtable);

    // This is to force the loading of the returned value before
    //  returning it, to avoid returning stale versions.
    if (result.memory_location != nullptr)
        result.set(nullptr);

    // The return statement has a void type. This is different from the type of
    //  the returned value, which is the type of the function call expression.
    return compiler->create_value(builder().CreateRet(result.get()), get_type());
}

}
