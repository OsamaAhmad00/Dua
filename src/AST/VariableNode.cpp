#include <AST/lvalue/VariableNode.hpp>
#include <types/PointerType.hpp>
#include <types/ReferenceType.hpp>

namespace dua
{

VariableNode::VariableNode(ModuleCompiler *compiler, std::string name, const Type *type)
        : name(std::move(name)), template_args({}), is_templated(false)
{
    this->compiler = compiler;
    this->type = type;
}

VariableNode::VariableNode(ModuleCompiler *compiler, std::string name, std::vector<const Type *> template_args, const Type *type)
        : name(std::move(name)), template_args(template_args), is_templated(true)
{
    this->compiler = compiler;
    this->type = type;
}

Value VariableNode::eval()
{
    if (is_templated) {
        // This is a templated function reference for sure since no identifier
        // is allowed to be templated, except for types and function reference.
        auto func = compiler->get_name_resolver().get_templated_function(name, template_args);
        // This returns a function type. If this is a function reference, we should
        //  wrap it in a pointer type.
        func.type = compiler->create_type<PointerType>(func.type);
        return func;
    }

    if (name_resolver().symbol_table.contains(name))
    {
        // This searches locally first, then globally if not found.
        // FIXME Symbol tables store the address in the loaded_value
        //  field, which is confusing. Store it in the memory_location
        //  field instead.
        auto result = name_resolver().symbol_table.get(name);
        if (auto ref = result.type->as<ReferenceType>(); ref != nullptr) {
            // If it's unallocated, then the address is stored in the loaded_value, and
            //  we're fine. If it's allocated, then we need to load the allocated variable
            //  first to get the desired address.
            if (ref->is_allocated()) {
                // This is an exception. LValue nodes should return assignable addresses.
                result.memory_location = result.get();
                result.memory_location = nullptr;
            }
        }
        result.type = get_type();
        return result;
    }

    // Not found. Has to be a function reference
    const FunctionType* func_type = nullptr;

    auto pointer_type = get_type()->as<PointerType>();
    if (pointer_type != nullptr)
        func_type = pointer_type->get_element_type()->as<FunctionType>();

    std::string full_name;
    if (func_type != nullptr)
        full_name = name_resolver().get_function_name_with_exact_type(name, func_type);
    else
        full_name = name_resolver().get_function_full_name(name);

    return compiler->create_value(module().getFunction(full_name), get_type());
}

const Type* VariableNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    if (is_templated) {
        // This is definitely a templated function reference.
        // Wrap the function type in a pointer
        return set_type(compiler->create_type<PointerType>(compiler->get_name_resolver().get_templated_function(name, template_args).type));
    }

    const Type* t = nullptr;

    if (name_resolver().symbol_table.contains(name)) {
        t = name_resolver().symbol_table.get(name).type;
    } else if (name_resolver().has_function(name)) {
        auto full_name = name_resolver().get_function_full_name(name);
        t = name_resolver().get_function_no_overloading(full_name).type;
    } else {
        report_error("The identifier " + name + " is not defined");
    }

    // References already point to some value. We just need to make
    //  sure that the returned reference type is an allocated reference.
    if (auto ref = t->as<ReferenceType>(); ref != nullptr)
        t = ref->get_allocated();
    else
        t = compiler->create_type<PointerType>(t);

    return set_type(t);
}

const Type *VariableNode::get_element_type()
{
    auto type = get_type();
    if (auto ref = type->as<ReferenceType>(); ref != nullptr)
        return ref->get_element_type();
    return ((PointerType*)get_type())->get_element_type();
}

bool VariableNode::is_function() const {
    return name_resolver().has_function(name);
}

std::string VariableNode::get_name() const {
    return name;
}

}
