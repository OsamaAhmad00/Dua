#include <AST/AssignmentNode.h>

llvm::StoreInst* AssignmentNode::eval()
{
    llvm::Value* ptr = symbol_table().get(lhs).ptr;
    return builder().CreateStore(rhs->eval(), ptr);
}

AssignmentNode::~AssignmentNode()
{
    delete rhs;
}