#include "AST/variable/LocalVariableDefinitionNode.hpp"

namespace dua
{

llvm::Constant* LocalVariableDefinitionNode::get_constant(llvm::Value* value, llvm::Type* target_type, const std::string& field_name)
{
    auto const_init = llvm::dyn_cast<llvm::Constant>(value);
    if (const_init == nullptr)
        report_error("Can't initialize class fields with a non-constant expression (in " + field_name + ")");

    auto casted = (llvm::Constant*)compiler->cast_value(const_init, target_type, false);

    if (casted == nullptr)
        report_error("Initializing the field " + field_name + " with a mismatching type expression");

    return casted;
}

llvm::Value* LocalVariableDefinitionNode::eval()
{
    auto full_name = (current_class() ? current_class()->getName().str() + "::" : "") + name;

    if (initializer != nullptr && !args.empty())
        report_error("Can't have both an initializer and an initializer list (in " + full_name + ")");

    llvm::Value* init_value = initializer ? initializer->eval() : type->default_value();
    if (current_function() != nullptr) {
        std::vector<llvm::Value*> llvm_args(args.size());
        for (int i = 0; i < args.size(); i++)
            llvm_args[i] = args[i]->eval();
        return create_local_variable(name, type, init_value, std::move(llvm_args));
    }

    if (current_class() == nullptr)
        report_internal_error("Local variable definition in a non-local scope");

    // A class is being processed at the moment
    // This is a field definition, not a local variable definition.

    auto class_name = current_class()->getName().str();
    if (name_resolver().has_function(class_name + "." + name))
        report_error("The identifier " + full_name + " is defined as both a field and a method");

    llvm::Constant* default_value = init_value ?
            get_constant(init_value, type->llvm_type(), full_name) : type->default_value();

    std::vector<llvm::Constant*> default_args;
    if (!args.empty()) {
        default_args.resize(args.size());
        for (size_t i = 0; i < args.size(); i++)
            default_args[i] = get_constant(args[i]->eval(), args[i]->get_cached_type()->llvm_type(), full_name);
    }

    name_resolver().get_class(class_name)->fields().push_back({
        name,
        type->clone(),
        default_value,
        std::move(default_args)
    });

    return none_value();
}

}
