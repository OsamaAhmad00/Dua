#include "AST/lvalue/ClassFieldNode.hpp"
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"

namespace dua
{

static const ClassType* get_class(const Type* type)
{
    auto casted = type->as<ClassType>();
    if (casted == nullptr)
        report_error("Member access on a non-class (" + type->to_string() + ") type");
    return casted;
}

const static ClassType* get_class_from_ptr(ASTNode* node)
{
    auto type = node->get_type();

    const Type* element_type = nullptr;
    if (auto ptr = type->as<PointerType>(); ptr != nullptr)
        element_type = ptr->get_element_type();
    else if (auto ref = type->as<ReferenceType>(); ref != nullptr)
        element_type = ref->get_element_type();
    else
        report_internal_error("Field access on a non-pointer (" + type->to_string() + ") type");

    return get_class(element_type);
}

Value ClassFieldNode::eval()
{

    auto full_name = get_name();

    if (is_templated) {
        // This is a templated method reference for sure since no identifier
        // is allowed to be templated, except for types and function reference.
        auto func = compiler->get_name_resolver().get_templated_function(full_name, template_args);
        // This returns a function type. If this is a function reference, we should
        //  wrap it in a pointer type.
        func.type = compiler->create_type<PointerType>(func.type);
        return func;
    }

    if (name_resolver().has_function(full_name)) {
        auto name = name_resolver().get_function_full_name(full_name);
        return compiler->create_value(module().getFunction(name), get_type());
    }

    auto class_type = get_class_from_ptr(instance);
    auto result = class_type->get_field(eval_instance(), name);
    if (auto ref = result.type->as<ReferenceType>(); ref != nullptr) {
        // Reference fields has two indirections instead of one.
        //  Normal references would just hold the address of the
        //  referenced variable, and getting the value would be
        //  a matter of dereferencing the address. For reference
        //  fields, first, dereference the offset in the class to
        //  get the pointer, then dereference the pointer to get
        //  the value. This is why we store into the memory_location,
        //  to get two dereferences instead of one
        result.memory_location = result.get();
        result.set(nullptr);
        // The returned reference is allocated, since after the value is
        //  loaded, it'll store the pointer, not the value itself
        result.type = ref->get_allocated();
    } else {
        result.type = compiler->create_type<PointerType>(result.type);
    }

    return result;
}

const Type* ClassFieldNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    auto full_name = get_name();

    if (is_templated) {
        return set_type(compiler->create_type<PointerType>(compiler->get_name_resolver().get_templated_function(full_name, template_args).type));
    }

    const Type* t;

    if (name_resolver().has_function(full_name)) {
        auto name = name_resolver().get_function_full_name(full_name);
        t = name_resolver().get_function_no_overloading(name).type;
    } else {
        auto class_type = get_class_from_ptr(instance);
        t = class_type->get_field(name).type;
    }

    // References already point to some value
    if (t->as<ReferenceType>() == nullptr)
        t = compiler->create_type<PointerType>(t);

    return set_type(t);
}

Value ClassFieldNode::eval_instance() const
{
    if (!instance_eval.is_null()) return instance_eval;
    return instance_eval = instance->eval();
}

std::string ClassFieldNode::get_name() const
{
    auto class_type = get_class_from_ptr(instance);
    return class_type->name + "." + name;
}

bool ClassFieldNode::is_function() const {
    return name_resolver().has_function(get_name());
}

}
