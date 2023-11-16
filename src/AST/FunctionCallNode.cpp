#include <AST/function/FunctionCallNode.h>

llvm::CallInst* FunctionCallNode::eval()
{
    llvm::Function* function = module().getFunction(name);
    auto& signature = compiler->get_function(name);

    std::vector<llvm::Value*> llvm_args(args.size());
    for (size_t i = args.size() - 1; i != (size_t)-1; i--) {
        llvm_args[i] = args[i]->eval();
        if (i < signature.params.size()) {
            // Only try to cast non-var-arg parameters
            auto param_type = signature.params[i].type->llvm_type();
            llvm_args[i] = compiler->cast_value(llvm_args[i], param_type);
        }
    }

    return builder().CreateCall(function, llvm_args);
}

FunctionCallNode::~FunctionCallNode()
{
    for (ASTNode* arg : args)
        delete arg;
}

TypeBase *FunctionCallNode::compute_type() {
    delete type;
    return type = compiler->get_function(name).return_type->clone();
}
