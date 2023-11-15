#include <AST/function/ReturnNode.h>
#include <types/VoidType.h>

llvm::ReturnInst* ReturnNode::eval()
{
    if (expression == nullptr)
        return builder().CreateRetVoid();
    return builder().CreateRet(expression->eval());
}

ReturnNode::~ReturnNode() { delete expression; }

