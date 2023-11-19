#include <AST/function/ReturnNode.h>
#include <types/VoidType.h>

namespace dua
{

llvm::ReturnInst* ReturnNode::eval()
{
    if (expression == nullptr)
        return builder().CreateRetVoid();
    auto result = expression->eval();
    auto casted = compiler->cast_value(result, current_function()->getReturnType());
    return builder().CreateRet(casted);
}

ReturnNode::~ReturnNode() { delete expression; }

}
