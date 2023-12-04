#include "AST/lvalue/DereferenceNode.hpp"
#include "types/PointerType.hpp"
#include <utils/ErrorReporting.hpp>

namespace dua
{

PointerType* assert_ptr(Type* type) {
    auto ptr = dynamic_cast<PointerType*>(type);
    if (!ptr) report_error("Can't dereference a non-pointer type expression");
    return ptr;
}

llvm::Value* DereferenceNode::eval()
{
    assert_ptr(address->get_cached_type());
    return address->eval();
}

Type *DereferenceNode::compute_type()
{
    delete type;
    return type = assert_ptr(address->get_cached_type()->clone());
}

Type* DereferenceNode::get_element_type()
{
    auto ptr = assert_ptr(get_cached_type());
    return ptr->get_element_type();
}

}
