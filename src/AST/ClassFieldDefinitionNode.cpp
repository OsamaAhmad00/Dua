#include <AST/variable/ClassFieldDefinitionNode.hpp>
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"


namespace dua
{

llvm::Constant* ClassFieldDefinitionNode::get_constant(Value value, const Type* target_type, const std::string& field_name)
{
    value.set(value.as<llvm::Constant>());
    if (value.is_null())
        report_error("Can't initialize class fields with a non-constant expression (in " + field_name + ")");

    auto casted = typing_system().cast_value(value, target_type, false).as<llvm::Constant>();

    if (casted == nullptr)
        report_error("Initializing the field " + field_name + " with a mismatching type expression");

    return casted;
}

NoneValue ClassFieldDefinitionNode::eval()
{
    auto full_name = (current_class() ? current_class()->getName().str() + "::" : "") + name;

    if (initializer != nullptr && !args.empty())
        report_error("Can't have both an initializer and an initializer list (in " + full_name + ")");

    Value init_value;
    if (initializer) init_value = initializer->eval();

    auto class_name = current_class()->getName().str();
    if (name_resolver().has_function(class_name + "." + name))
        report_error("The identifier " + full_name + " is defined as both a field and a method");

    llvm::Constant* default_value = nullptr;
    if (initializer != nullptr) {
        auto as_ref = this->type->as<ReferenceType>();
        if (as_ref != nullptr) {
            assert(init_value.memory_location != nullptr);
            init_value.set(init_value.memory_location);
        }
        default_value = get_constant(init_value, type, full_name);
    }

    std::vector<Value> default_args;
    if (!args.empty())
    {
        default_args.resize(args.size());
        for (size_t i = 0; i < args.size(); i++)
        {
            auto arg = args[i]->eval();
            if (type->as<ReferenceType>() != nullptr) {
                assert(arg.memory_location != nullptr);
                arg.set(arg.memory_location);
            }

            arg.set(get_constant(
                arg,
                arg.type,
                full_name
            ));

            default_args[i] = arg;
        }
    }

    // non-const direct access
    for (auto& field : name_resolver().class_fields[class_name]) {
        if (field.name == name) {
            // The field type can be different type in case of fields
            // with templated types, in which field.type will be a
            // concrete type and type will be a templated type
            field.default_value = default_value;
            field.default_args = std::move(default_args);
            break;
        }
    }

    return none_value();
}

}