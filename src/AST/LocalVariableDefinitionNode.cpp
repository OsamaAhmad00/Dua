#include "AST/variable/LocalVariableDefinitionNode.hpp"

namespace dua
{

llvm::Constant* LocalVariableDefinitionNode::get_constant(Value value, const Type* target_type, const std::string& field_name)
{
    value.ptr = llvm::dyn_cast<llvm::Constant>(value.ptr);
    if (value.ptr == nullptr)
        report_error("Can't initialize class fields with a non-constant expression (in " + field_name + ")");

    auto casted = (llvm::Constant*)typing_system().cast_value(value, target_type, false);

    if (casted == nullptr)
        report_error("Initializing the field " + field_name + " with a mismatching type expression");

    return casted;
}

llvm::Value* LocalVariableDefinitionNode::eval()
{
    auto full_name = (current_class() ? current_class()->getName().str() + "::" : "") + name;

    if (initializer != nullptr && !args.empty())
        report_error("Can't have both an initializer and an initializer list (in " + full_name + ")");

    Value init_value = initializer
            ? compiler->create_value(initializer->eval(), initializer->get_type())
            : compiler->create_value(type->default_value(), type);
    if (current_function() != nullptr) {
        std::vector<Value> evaluated(args.size());
        for (int i = 0; i < args.size(); i++)
            evaluated[i] = compiler->create_value(args[i]->eval(), args[i]->get_type());
        return create_local_variable(name, type, &init_value, std::move(evaluated));
    }

    if (current_class() == nullptr)
        report_internal_error("Local variable definition in a non-local scope");

    // A class is being processed at the moment
    // This is a field definition, not a local variable definition.

    auto class_name = current_class()->getName().str();
    if (name_resolver().has_function(class_name + "." + name))
        report_error("The identifier " + full_name + " is defined as both a field and a method");

    llvm::Constant* default_value = initializer ?
            get_constant(init_value, type, full_name) : type->default_value();

    std::vector<Value> default_args;
    if (!args.empty()) {
        default_args.resize(args.size());
        for (size_t i = 0; i < args.size(); i++)
        {
            auto constant = get_constant(
                    compiler->create_value(args[i]->eval(), args[i]->get_type()),
                    args[i]->get_type(),
                    full_name
            );

            default_args[i] = compiler->create_value(constant, args[i]->get_type());
        }
    }

    // non-const direct access
    name_resolver().class_fields[class_name].push_back({
        name,
        type,
        default_value,
        std::move(default_args)
    });

    return none_value();
}

}
