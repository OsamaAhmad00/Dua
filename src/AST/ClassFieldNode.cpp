#include "AST/lvalue/ClassFieldNode.hpp"
#include "types/PointerType.hpp"

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
    auto ptr = dynamic_cast<const PointerType*>(type);
    if (ptr == nullptr)
        report_internal_error("Field access on a non-pointer (" + type->to_string() + ") type");
    return get_class(ptr->get_element_type());
}

Value ClassFieldNode::eval()
{
    auto full_name = get_full_name();
    if (name_resolver().has_function(full_name)) {
        auto name = name_resolver().get_function(full_name);
        return compiler->create_value(module().getFunction(name), get_type());
    }

    auto class_type = get_class_from_ptr(instance);
    return compiler->create_value(class_type->get_field(eval_instance(), name).ptr, get_type());
}

const Type* ClassFieldNode::get_type()
{
    if (type != nullptr) return type;
    auto full_name = get_full_name();
    const Type* t;
    if (name_resolver().has_function(full_name)) {
        auto name = name_resolver().get_function(full_name);
        t = name_resolver().get_function_no_overloading(name).type;
    } else {
        auto class_type = get_class_from_ptr(instance);
        t = class_type->get_field(name).type;
    }
    return type = compiler->create_type<PointerType>(t);
}

Value ClassFieldNode::eval_instance() const
{
    if (instance_eval.ptr != nullptr) return instance_eval;
    return instance_eval = instance->eval();
}

std::string ClassFieldNode::get_full_name() const
{
    auto class_type = get_class_from_ptr(instance);
    return class_type->name + "." + name;
}

bool ClassFieldNode::is_function() const {
    return name_resolver().has_function(get_full_name());
}

}
