#include <AST/function/FunctionCallNode.h>

namespace dua
{

llvm::CallInst* FunctionCallNode::eval()
{
    std::vector<llvm::Value*> llvm_args(args.size());
    for (size_t i = args.size() - 1; i != (size_t)-1; i--)
        llvm_args[i] = args[i]->eval();
    return compiler->call_function(name, llvm_args);
}

TypeBase *FunctionCallNode::compute_type() {
    delete type;
    return type = compiler->get_function(name).return_type->clone();
}

FunctionCallNode::~FunctionCallNode() {
    for (ASTNode* arg : args)
        delete arg;
}

}