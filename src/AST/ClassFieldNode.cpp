#include "AST/lvalue/ClassFieldNode.hpp"
#include "types/PointerType.hpp"

namespace dua
{

static ClassType* get_class(Type* type)
{
    auto casted = dynamic_cast<ClassType*>(type);
    if (casted == nullptr)
        report_error("Member access on a non-class (" + type->to_string() + ") type");
    return casted;
}

static ClassType* get_class_from_ptr(ASTNode* node)
{
    auto type = node->get_cached_type();
    auto ptr = dynamic_cast<PointerType*>(type);
    if (ptr == nullptr)
        report_internal_error("Field access on a non-pointer (" + type->to_string() + ") type");
    return get_class(ptr->get_element_type());
}

llvm::Value* ClassFieldNode::eval()
{
    auto full_name = get_full_name();
    if (name_resolver().has_function(full_name))
        return module().getFunction(full_name);

    auto class_type = get_class_from_ptr(instance);
    return class_type->get_field(get_instance(), name);
}

Type* ClassFieldNode::compute_type()
{
    delete type;
    auto full_name = get_full_name();
    Type* t;
    if (name_resolver().has_function(full_name))
        t = name_resolver().get_function(full_name).type.clone();
    else {
        auto class_type = get_class_from_ptr(instance);
        t = class_type->get_field(name).type->clone();
    }
    return type = compiler->create_type<PointerType>(t);
}

llvm::Value *ClassFieldNode::get_instance()
{
    if (instance_eval != nullptr) return instance_eval;
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
