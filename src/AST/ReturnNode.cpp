#include <AST/function/ReturnNode.hpp>
#include <types/VoidType.hpp>

namespace dua
{

llvm::ReturnInst* ReturnNode::eval()
{
    if (expression == nullptr)
        return builder().CreateRetVoid();
    auto result = compiler->create_value(expression->eval(), expression->get_type());
    auto return_type = name_resolver().get_function_no_overloading(
            current_function()->getName().str()).type->return_type;
    auto casted = typing_system().cast_value(result, return_type);
    return builder().CreateRet(casted);
}

}
