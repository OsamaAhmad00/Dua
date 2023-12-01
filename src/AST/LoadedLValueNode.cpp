#include <AST/lvalue/LoadedLValueNode.h>
#include <types/PointerType.h>

namespace dua
{

llvm::Value* LoadedLValueNode::eval()
{
    auto ptr = lvalue->eval();

    // If it's pointing to a function, don't load the address
    auto element_type = lvalue->get_element_type();
    if (dynamic_cast<FunctionType*>(element_type) != nullptr)
        return ptr;

    return builder().CreateLoad(element_type->llvm_type(), ptr);
}

TypeBase *LoadedLValueNode::compute_type()
{
    delete type;
    auto result = lvalue->get_cached_type();
    auto ptr = dynamic_cast<PointerType*>(result);
    assert(ptr != nullptr);

    // If it's pointing to a function, don't load the address
    auto element_type = lvalue->get_element_type();
    if (dynamic_cast<FunctionType*>(element_type) != nullptr)
        return type = ptr->clone();

    return type = ptr->get_element_type()->clone();
}

}
