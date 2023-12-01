#include "AST/lvalue/ClassFieldNode.h"
#include "types/PointerType.h"

namespace dua
{

struct FieldInfo
{
    size_t index;
    ClassField& info;
};

ClassType* get_class(Type* type)
{
    auto casted = dynamic_cast<ClassType*>(type);
    if (casted == nullptr)
        report_error("Member access on a non-class type");
    return casted;
}

FieldInfo get_field(Type* type, const std::string& field)
{
    auto casted = get_class(type);
    auto& fields = casted->fields();
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].name == field) {
            return { i, fields[i] };
        }
    }

    report_error("Class " + casted->name + " doesn't contain a member with the name " + field);

    // Unreachable
    return { 0, fields[0] };
}

llvm::Value* ClassFieldNode::eval()
{
    auto full_name = get_full_name();
    if (compiler->has_function(full_name))
        return module().getFunction(full_name);

    auto class_type = instance->get_element_type();
    auto field = get_field(class_type, name);
    return builder().CreateStructGEP(class_type->llvm_type(), get_instance(), field.index, name);
}

Type* ClassFieldNode::compute_type()
{
    delete type;
    auto full_name = get_full_name();
    Type* t;
    if (compiler->has_function(full_name))
        t = compiler->get_function(full_name).type.clone();
    else {
        auto class_type = instance->get_element_type();
        t = get_field(class_type, name).info.type->clone();
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
    auto class_type = get_class(instance->get_element_type());
    return class_type->name + "." + name;
}

bool ClassFieldNode::is_function() const {
    return compiler->has_function(get_full_name());
}

}
