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

    // Reserve a location for the self parameter
    if (!is_callable_field)
        args.insert(args.begin(), nullptr);

    processed = true;
}

Value MethodCallNode::eval()
{
    process();

    auto args = eval_args();

    auto class_type = get_instance_type();

    // This is an allocated reference, a change in the type
    //  to pointer is sufficient to turn it into a pointer value.
    Value instance_ptr = is_callable_field ? get_instance_ref() : args[0];
    instance_ptr.type = compiler->create_type<PointerType>(class_type);

    if (is_callable_field)
        return call_reference(class_type->get_field(instance_ptr, name), std::move(args));

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

    auto method = class_type->get_method(name, instance_ptr, arg_types);

    return name_resolver().call_function(method, std::move(args));
}

const Type* MethodCallNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    process();

    auto class_type = get_instance_type();
    auto arg_types = get_arg_types();
    auto full_name = name_resolver().get_winning_method(class_type, name, arg_types);

    if (is_templated) {
        std::swap(name, full_name);
        auto method_type = get_templated_function(arg_types);
        std::swap(name, full_name);
        return set_type(method_type.type->as<FunctionType>()->return_type);
    }

    auto method_type = name_resolver().get_function(std::move(full_name), std::move(arg_types)).type;
    return set_type(method_type->return_type);
}

std::vector<const Type*> MethodCallNode::get_arg_types()
{
    process();

    std::vector<const Type*> arg_types(args.size());

    arg_types[0] = compiler->create_type<ReferenceType>(get_instance_type(), true);

    for (size_t i = 1; i < args.size(); i++)
        if (args[i] != nullptr)
            arg_types[i] = args[i]->get_type();

    return arg_types;
}

const ClassType *MethodCallNode::get_instance_type()
{
    auto instance_type = instance_node->get_type();

    auto class_type = instance_type->get_contained_type()->as<ClassType>();
    if (class_type == nullptr)
        compiler->report_error("Can't call a method " + name + " from the type " + instance_type->to_string() + ", which is not of a class type");

    return class_type;
}

Value MethodCallNode::get_instance_ref()
{
    // FIXME different methods expect different types. Some expect a reference
    //  and some expect a pointer type. Also, different instance nodes return
    //  different instance types. Make a uniform interface for all of them

    auto instance = instance_node->eval();

    Value as_ref;
    if (auto ref = instance.type->as<ReferenceType>(); ref != nullptr) {
        as_ref = instance;
        if (!ref->is_allocated()) {
            as_ref.set(as_ref.memory_location);
            as_ref.type = ref->get_allocated();
        }
    } else {
        assert(instance.memory_location != nullptr);
        as_ref.set(instance.memory_location);
        as_ref.type = compiler->create_type<ReferenceType>(instance.type, true);
    }

    return as_ref;
}

std::vector<Value> MethodCallNode::eval_args()
{
    auto args = FunctionCallNode::eval_args();

    // Don't include the self parameter if it's a callable field
    if (is_callable_field) return args;

    args[0] = get_instance_ref();

    return args;
}

}