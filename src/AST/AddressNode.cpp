#include "AST/lvalue/AddressNode.h"
#include "types/PointerType.h"

namespace dua
{

llvm::Value* AddressNode::eval()
{
    auto ptr = address->eval();

    if (!load_value)
        return ptr;

    auto t = get_cached_type()->llvm_type();

    return builder().CreateLoad(t, ptr);
}

TypeBase *AddressNode::compute_type()
{
    delete type;

    TypeBase* t = address->get_cached_type();
    auto ptr = dynamic_cast<PointerType*>(t);

    if (ptr == nullptr)
        throw std::runtime_error("Can't dereference a non-pointer type");

    return type = (load_value ? ptr->get_element_type() : ptr)->clone();
}

}
