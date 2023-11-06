#include <AST/FunctionCallNode.h>

llvm::CallInst* FunctionCallNode::eval()
{
    llvm::Function* function = module().getFunction(name);

    std::vector<llvm::Value*> llvm_args(args.size());
    for (int i = 1; i < args.size(); i++)
        llvm_args[i] = args[i]->eval();

    return builder().CreateCall(function, llvm_args);
}