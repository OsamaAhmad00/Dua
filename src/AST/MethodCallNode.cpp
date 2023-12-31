#include <AST/function/MethodCallNode.hpp>
#include <types/ReferenceType.hpp>
#include <AST/lvalue/VariableNode.hpp>
#include "types/PointerType.hpp"

namespace dua
{

void MethodCallNode::process()
{
    if (processed)
        return;

    auto class_type = get_instance_type();

    is_callable_field = false;
    for (auto& field : class_type->fields()) {
        if (field.name == name) {
            is_callable_field = true;
            break;
        }
    }

    if (!is_callable_field)
        args.insert(args.begin(), instance_node);

    processed = true;
}

Value MethodCallNode::eval()
{
    process();

    auto instance = instance_node->eval();

    auto class_type = get_instance_type();

    if (is_callable_field)
        return call_reference(class_type->get_field(instance, name), eval_args());

    auto args = eval_args();
    // Reference types act as the same type as the llvm type they're referencing.
    // This is a quick hack, in which the instance is loaded as an llvm pointer
    //  first, then we change its type to be a reference type
    args[0].get();
    args[0].type = compiler->create_type<ReferenceType>(class_type);

    if (is_templated)
    {
        // Templated virtual methods are not supported. If the method is
        //  templated, call the method without dynamic dispatch.
        auto old_name = name;
        name = class_type->name + "." + name;
        auto result = call_templated_function(std::move(args));
        name = old_name;
        return result;
    }

    std::vector<const Type*> arg_types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        arg_types[i] = args[i].type;

    // Load the symbol table. The symbol table may be of a child class.
    // The type tho, will be considered of the current class
    auto vtable = name_resolver().get_vtable_instance(class_type->name);
    auto vtable_ptr_ptr = class_type->get_field(instance, ".vtable_ptr");
    auto vtable_type = name_resolver().get_vtable_type(class_type->name)->llvm_type();
    auto vtable_ptr = builder().CreateLoad(vtable_type, vtable_ptr_ptr.get(), ".vtable");
    auto full_name = name_resolver().get_winning_method(class_type, name, arg_types);
    auto method_type = name_resolver().get_function(full_name, args).type;
    auto method_ptr = vtable->get_method(full_name, method_type->llvm_type()->getPointerTo(), vtable_ptr);
    auto method = compiler->create_value(method_ptr, method_type);

    return name_resolver().call_function(method, std::move(args));
}

const Type* MethodCallNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    process();

    auto instance_type = instance_node->get_type();
    auto class_type = instance_type->as<ClassType>();
    if (class_type == nullptr)
        report_error("Can't call a method " + name + " from the type " + instance_type->to_string() + ", which is not of a class type");
    auto method_type = name_resolver().get_function(class_type->name + "." + name, get_arg_types()).type;

    return set_type(method_type->return_type);
}

std::vector<const Type*> MethodCallNode::get_arg_types()
{
    process();

    std::vector<const Type*> arg_types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        arg_types[i] = args[i]->get_type();

    auto ptr_type = arg_types[0]->as<PointerType>();
    assert(ptr_type != nullptr);
    arg_types[0] = compiler->create_type<ReferenceType>(ptr_type->get_element_type());

    return arg_types;
}

const ClassType *MethodCallNode::get_instance_type()
{
    auto instance_type = instance_node->get_type();
    auto ptr_type = instance_type->as<PointerType>();
    if (ptr_type == nullptr)
        report_error(instance_type->to_string() + " is not an lvalue type, and has no method with the name " + name);
    auto element_type = ptr_type->get_element_type();
    auto class_type = element_type->get_contained_type()->as<ClassType>();
    if (class_type == nullptr)
        report_error(element_type->to_string() + " is not of a class type, and has no method with the name " + name);
    return class_type;
}

}