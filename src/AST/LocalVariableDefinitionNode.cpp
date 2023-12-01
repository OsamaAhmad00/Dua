#include "AST/variable/LocalVariableDefinitionNode.h"

namespace dua
{

llvm::Value* LocalVariableDefinitionNode::eval()
{
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

    if (compiler->has_function(class_name + "." + name))
        report_error("The identifier " + class_name + "::" + name + " is defined as both a field and a method");

    llvm::Constant* const_init = nullptr;
    if (init_value) {
        const_init = llvm::dyn_cast<llvm::Constant>(init_value);
        if (const_init == nullptr)
            report_error("Can't initialize class fields with a non-constant expression");
    }

    auto default_value = const_init
            ? (llvm::Constant*)compiler->cast_value(const_init, type->llvm_type())
            : type->default_value();

    compiler->get_class(class_name)->fields().push_back({
        name,
        type->clone(),
        default_value
    });

    return none_value();
}

}
