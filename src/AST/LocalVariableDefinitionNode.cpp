#include <AST/LocalVariableDefinitionNode.h>

namespace dua
{

llvm::Value *LocalVariableDefinitionNode::eval()
{
    llvm::Value* init_value = initializer ? initializer->eval() : type->default_value();
    if (current_function() != nullptr)
        return create_local_variable(name, type, init_value);

    if (current_class() == nullptr)
        report_internal_error("Local variable definition in a non-local scope");

    // A class is being processed at the moment

    llvm::Constant* const_init = nullptr;
    if (init_value) {
        const_init = llvm::dyn_cast<llvm::Constant>(init_value);
        if (const_init == nullptr)
            report_error("Can't initialize class fields with a non-constant expression");
    }

    auto class_name = current_class()->getName().str();
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

LocalVariableDefinitionNode::~LocalVariableDefinitionNode()
{
    delete initializer;
}

}
