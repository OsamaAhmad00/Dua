#include <AST/function/ReturnNode.hpp>
#include <types/VoidType.hpp>

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
