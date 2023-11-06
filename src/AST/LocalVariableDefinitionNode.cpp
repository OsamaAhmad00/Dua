#include <AST/LocalVariableDefinitionNode.h>

llvm::AllocaInst *LocalVariableDefinitionNode::eval()
{
    return create_local_variable(name, type->llvm_type(), initializer->eval());
}

LocalVariableDefinitionNode::~LocalVariableDefinitionNode()
{
    delete initializer;
}