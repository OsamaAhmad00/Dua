#include <AST/lvalue/LoadedLValueNode.hpp>
#include <types/PointerType.hpp>

namespace dua
{

llvm::Value* LoadedLValueNode::eval()
{
    auto ptr = lvalue->eval();

    // If it's pointing to a function, don't load the address
    auto element_type = lvalue->get_element_type();
    if (dynamic_cast<const FunctionType*>(element_type) != nullptr)
        return ptr;

    return builder().CreateLoad(element_type->llvm_type(), ptr);
}

const Type *LoadedLValueNode::get_type()
{
    if (type != nullptr) return type;
    auto result = lvalue->get_type();
    auto ptr = dynamic_cast<const PointerType*>(result);
    assert(ptr != nullptr);

    // If it's pointing to a function, don't load the address
    auto element_type = lvalue->get_element_type();
    if (dynamic_cast<const FunctionType*>(element_type) != nullptr)
        return type = ptr;

    return type = ptr->get_element_type();
}

}
