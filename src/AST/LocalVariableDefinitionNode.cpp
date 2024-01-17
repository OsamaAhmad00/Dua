#include "AST/variable/LocalVariableDefinitionNode.hpp"
#include "types/PointerType.hpp"
#include "AST/lvalue/LoadedLValueNode.hpp"
#include "types/ReferenceType.hpp"

namespace dua
{

Value LocalVariableDefinitionNode::eval()
{
    auto full_name = (current_class() ? current_class()->getName().str() + "::" : "") + name;

    if (initializer != nullptr && !args.empty())
        report_error("Can't have both an initializer and an initializer list (in " + full_name + ")");

    // If it's a reference, just add a record to the symbol table and return
    if (auto ref = type->as<ReferenceType>(); ref != nullptr) {
        if (auto loaded = initializer->as<LoadedLValueNode>(); loaded != nullptr) {
            // The symbol table takes the type of the element type, not the pointer type.
            auto result = loaded->lvalue->eval();
            // It must be a pointer
            auto ptr_type = result.type->as<PointerType>();
            auto element_type = ptr_type->get_element_type();
            if (!typing_system().is_castable(element_type, type))
                report_error("Can't have a reference of type " + type->to_string() + " to an instance of type " + element_type->to_string());
            // Here, we perform the optimization of turning allocated reference into an unallocated one
            // FIXME the symbol table holds addresses of variables in the loaded_value field,
            //  instead of in the memory_location field. This means that we need not put the
            //  address of the variable in the memory_location field. This is done through
            //  all usages of the symbol table. Change it so that the address is stored in
            //  the memory_location field.
            result.type = ref->get_unallocated();
            name_resolver().symbol_table.insert(name, result);
            return result;
        } else {
            report_error("Can't have a reference type to a non-lvalue expression");
        }
    }

    // TODO don't evaluate if not going to be used
    Value init_value;
    if (initializer) init_value = initializer->eval();

    if (current_function() != nullptr)
    {
        std::vector<Value> evaluated(args.size());
        for (int i = 0; i < args.size(); i++)
            evaluated[i] = args[i]->eval();
        auto ptr = initializer ? &init_value : nullptr;
        auto alloc = create_local_variable(name, type, ptr, std::move(evaluated));
        return compiler->create_value(alloc, get_type());
    }
    else if (current_class() == nullptr)
    {
        report_internal_error("Local variable definition in a non-local scope (the variable " + full_name + ")");
        return none_value();
    }
    else
    {
        report_internal_error("Local variable definition in place of a field definition");
        return none_value();
    }
}

}
