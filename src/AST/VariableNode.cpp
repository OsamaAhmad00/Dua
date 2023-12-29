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

    if (name_resolver().symbol_table.contains(name)) {
        // This searches locally first, then globally if not found.
        return compiler->create_value(name_resolver().symbol_table.get(name).get(), get_type());
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
    if (type != nullptr) return type;

    if (is_templated) {
        // This is definitely a templated function reference.
        // Wrap the function type in a pointer
        return set_type(compiler->create_type<PointerType>(compiler->get_name_resolver().get_templated_function(name, template_args).type));
    }

    const Type* t;

    if (name_resolver().symbol_table.contains(name)) {
        t = name_resolver().symbol_table.get(name).type;
    } else if (name_resolver().has_function(name)) {
        auto full_name = name_resolver().get_function_full_name(name);
        t = name_resolver().get_function_no_overloading(full_name).type;
    } else {
        report_error("The identifier " + name + " is not defined");
    }

    return set_type(compiler->create_type<PointerType>(t));
}

const Type *VariableNode::get_element_type()
{
    return ((PointerType*)get_type())->get_element_type();
}

bool VariableNode::is_function() const {
    return name_resolver().has_function(name);
}

}
