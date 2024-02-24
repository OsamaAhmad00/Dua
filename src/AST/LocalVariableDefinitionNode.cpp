#include "AST/variable/LocalVariableDefinitionNode.hpp"
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"
#include "AST/IndexingNode.hpp"

namespace dua
{

Value LocalVariableDefinitionNode::eval()
{
    auto full_name = (current_class() ? current_class()->getName().str() + "::" : "") + name;

    if (current_function() == nullptr)
    {
        if (current_class() == nullptr)
            compiler->report_internal_error("Local variable definition in a non-local scope (the variable " + full_name + ")");
        else
            compiler->report_internal_error("Local variable definition in place of a field definition");
    }

    if (initializer != nullptr && !args.empty())
        compiler->report_error("Can't have both an initializer and an initializer list (in " + full_name + ")");

    auto ref = type->as<ReferenceType>();

    // If it's a reference, just add a record to the symbol table and return
    if (ref != nullptr)
    {
        if (initializer == nullptr)
            compiler->report_error("The reference variable " + name + " with type " + get_type()->to_string() + " must be assigned a variable to reference");

        auto result = initializer->eval();

        auto i_ref = result.type->as<ReferenceType>();

        if (result.is_teleporting && !i_ref) {
            compiler->report_error("The reference variable " + name + " of type " + type->to_string() +
                         " can't be bound to a temporary variable of type " + result.type->to_string() +
                         " which will be stale by the next statement");
        }

        if (i_ref != nullptr && i_ref->is_allocated()) {
            result.memory_location = result.get();
            result.set(nullptr);
            result.type = i_ref->get_unallocated();
        }

        if (result.memory_location == nullptr)
            compiler->report_error("Can't have a reference type to a non-lvalue expression");

        result.set(result.memory_location);

        if (!typing_system().is_castable(result.type, type))
            compiler->report_error("Can't have a reference of type " + type->to_string() + " to an instance of type " + result.type->to_string());
        // Here, we perform the optimization of turning allocated reference into an unallocated one
        // FIXME the symbol table holds addresses of variables in the loaded_value field,
        //  instead of in the memory_location field. This means that we need not put the
        //  address of the variable in the memory_location field. This is done through
        //  all usages of the symbol table. Change it so that the address is stored in
        //  the memory_location field.
        // The symbol table takes the type of the element type, not the pointer type.
        result.type = ref->get_unallocated();
        name_resolver().symbol_table.insert(name, result);
        return result;
    }

    Value init_value;
    if (initializer) {
        init_value = initializer->eval();
    }

    auto ptr = initializer ? &init_value : nullptr;

    compiler->create_local_variable(name, type, ptr, eval_args(args));

    return none_value();
}

}
