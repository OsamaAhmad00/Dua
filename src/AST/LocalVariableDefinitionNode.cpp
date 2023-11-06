#include <AST/LocalVariableDefinitionNode.h>

llvm::AllocaInst *LocalVariableDefinitionNode::eval()
{
    llvm::Type* llvm_type = get_type(type);
    return create_local_variable(name, llvm_type, initializer->eval());
}

LocalVariableDefinitionNode::~LocalVariableDefinitionNode()
{
    delete initializer;
}