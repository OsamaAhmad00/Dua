#include <AST/function/ReturnNode.hpp>
#include <types/VoidType.hpp>
#include "types/PointerType.hpp"

namespace dua
{

Value ReturnNode::eval()
{
    if (expression == nullptr)
        return compiler->create_value(builder().CreateRetVoid(), compiler->create_type<VoidType>());

    auto func = current_function()->getName().str();
    auto result = expression->eval();
    auto return_type = name_resolver().get_function_no_overloading(func).type->return_type;

    llvm::Value* ptr;
    if (auto ref = return_type->as<ReferenceType>(); ref != nullptr) {
        auto ptr_type = compiler->create_type<PointerType>(result.type);
        auto mem_location = compiler->create_value(result.memory_location, ptr_type);
        ptr = typing_system().cast_value(mem_location, ptr_type).get();
    } else {
        ptr = typing_system().cast_value(result, return_type).get();
    }

    // Destruct the objects before returning
    compiler->destruct_function_scope();

    return compiler->create_value(builder().CreateRet(ptr), get_type());
}

}
