#include "AST/lvalue/ClassFieldNode.h"
#include "types/PointerType.h"

namespace dua
{

struct FieldInfo
{
    size_t index;
    ClassField& info;
};

FieldInfo get_field(TypeBase* type, const std::string& field)
{
    auto casted = dynamic_cast<ClassType*>(type);
    if (casted == nullptr)
        report_error("Field access on a non-class type");

    auto& fields = casted->fields();
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].name == field) {
            return { i, fields[i] };
        }
    }

    report_error("Class " + casted->name + " doesn't contain a field with the name " + field);
    // Unreachable
    return { 0, fields[0] };
}

llvm::Value* ClassFieldNode::eval()
{
    auto class_type = instance->get_element_type();
    auto field = get_field(class_type, field_name);
    return builder().CreateStructGEP(class_type->llvm_type(), instance->eval(), field.index, field_name);
}

TypeBase *ClassFieldNode::compute_type() {
    return type = compiler->create_type<PointerType>(get_element_type());
}

TypeBase *ClassFieldNode::get_element_type() {
    auto class_type = instance->get_element_type();
    auto field = get_field(class_type, field_name);
    return field.info.type;
}

}
