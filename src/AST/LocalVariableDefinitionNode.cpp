#include <AST/LocalVariableDefinitionNode.h>

namespace dua
{

llvm::AllocaInst *LocalVariableDefinitionNode::eval()
{
    llvm::Value* init_value = initializer ? initializer->eval() : nullptr;
    return create_local_variable(name, type, init_value);
}

LocalVariableDefinitionNode::~LocalVariableDefinitionNode()
{
    delete initializer;
}

}
