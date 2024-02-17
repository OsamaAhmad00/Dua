#include <AST/variable/ClassFieldDefinitionNode.hpp>
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"


namespace dua
{

NoneValue ClassFieldDefinitionNode::eval()
{
    auto full_name = (current_class() ? current_class()->getName().str() + "::" : "") + name;

    if (initializer != nullptr && !args.empty())
        compiler->report_error("Can't have both an initializer and an initializer list (in " + full_name + ")");

    auto class_name = current_class()->getName().str();
    if (name_resolver().has_function(class_name + "." + name))
        compiler->report_error("The identifier " + full_name + " is defined as both a field and a method");

    // non-const direct access
    for (auto& field : name_resolver().class_fields[class_name]) {
        if (field.name == name) {
            // The field type can be different type in case of fields
            // with templated types, in which field.type will be a
            // concrete type and type will be a templated type
            field.initializer = initializer;
            field.init_args = args;
            break;
        }
    }

    return none_value();
}

}