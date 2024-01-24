#include "AST/lvalue/DereferenceNode.hpp"
#include "types/PointerType.hpp"
#include <utils/ErrorReporting.hpp>

namespace dua
{

const PointerType* DereferenceNode::assert_ptr(const Type* type) {
    auto ptr = type->as<PointerType>();
    if (!ptr) compiler->report_error("Can't dereference a non-pointer type expression");
    return ptr;
}

Value DereferenceNode::eval()
{
    assert_ptr(address->get_type());
    return address->eval();
}

const Type* DereferenceNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    return set_type(assert_ptr(address->get_type()));
}

const Type* DereferenceNode::get_element_type()
{
    auto ptr = assert_ptr(get_type());
    return ptr->get_element_type();
}

}
