#include "AST/lvalue/DereferenceNode.hpp"
#include "types/PointerType.hpp"
#include <utils/ErrorReporting.hpp>

namespace dua
{

const PointerType* assert_ptr(const Type* type) {
    auto ptr = dynamic_cast<const PointerType*>(type);
    if (!ptr) report_error("Can't dereference a non-pointer type expression");
    return ptr;
}

Value DereferenceNode::eval()
{
    assert_ptr(address->get_type());
    return address->eval();
}

const Type* DereferenceNode::get_type()
{
    if (type != nullptr) return type;
    return type = assert_ptr(address->get_type());
}

const Type* DereferenceNode::get_element_type()
{
    auto ptr = assert_ptr(get_type());
    return ptr->get_element_type();
}

}
