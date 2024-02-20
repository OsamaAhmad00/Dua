#include "AST/lvalue/ClassFieldNode.hpp"
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"
#include <resolution/IdentityResolutionString.hpp>

namespace dua
{

const ClassType* ClassFieldNode::get_class(const Type* type) const
{
    auto casted = type->get_contained_type()->as<ClassType>();
    if (casted == nullptr)
        compiler->report_error("Member access on a non-class (" + type->to_string() + ") type");
    return casted;
}

Value ClassFieldNode::eval()
{
    auto name = unresolved_name->resolve();

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

    auto i = instance->eval();
    if (i.memory_location == nullptr)
        report_internal_error("Can't access a field (with name " + name + ") from an expression with type " + instance->get_type()->to_string());

    auto class_type = get_class(i.type);
    auto instance_ptr = compiler->create_value(i.memory_location, compiler->create_type<ReferenceType>(class_type, true));

    // get_field returns a pointer. Turn it into value
    auto result = class_type->get_field(instance_ptr, name);
    result.memory_location = result.get();
    result.set(nullptr);

    if (auto ref = result.type->as<ReferenceType>(); ref != nullptr) {
        // Reference fields has two indirections instead of one.
        //  Normal references would just hold the address of the
        //  referenced variable, and getting the value would be
        //  a matter of dereferencing the address. For reference
        //  fields, first, dereference the offset in the class to
        //  get the pointer, then dereference the pointer to get
        //  the value. To achieve this double load effect, we set
        //  the type of the reference to an allocated reference,
        //  so that components loading the value of the reference
        //  field know that what they have is an address, and not
        //  the value of the reference field.
        result.type = ref->get_allocated();
    }

    return result;
}

const Type* ClassFieldNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    auto full_name = get_name();

    if (is_templated) {
        // No field can be templated. This is a templated method
        auto func_type = compiler->get_name_resolver().get_templated_function(full_name, template_args).type;
        return set_type(compiler->create_type<PointerType>(func_type));
    }

    const Type* t;

    if (name_resolver().has_function(full_name)) {
        auto name = name_resolver().get_function_full_name(full_name);
        t = name_resolver().get_function_no_overloading(name).type;
        t = compiler->create_type<PointerType>(t);
    } else {
        auto class_type = get_class(instance->get_type());
        auto name = unresolved_name->resolve();
        t = class_type->get_field(name).type;
        t = compiler->create_type<ReferenceType>(t, true);
    }

    return set_type(t);
}

std::string ClassFieldNode::get_name() const
{
    auto name = unresolved_name->resolve();
    auto class_type = get_class(instance->get_type());
    return class_type->name + "." + name;
}

bool ClassFieldNode::is_function() const {
    return name_resolver().has_function(get_name());
}

ResolutionString *ClassFieldNode::get_resolution_name(ModuleCompiler *compiler, std::string name) {
    return compiler->get_name_resolver().create_resolution_string<IdentityResolutionString>(std::move(name));
}

}
