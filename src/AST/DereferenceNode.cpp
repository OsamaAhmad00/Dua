#include "AST/lvalue/DereferenceNode.hpp"
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"

namespace dua
{

const PointerType* DereferenceNode::assert_ptr(const Type* type) {
    auto ptr = type->as<PointerType>();
    if (!ptr)
        compiler->report_error("Can't dereference a non-pointer type expression");
    return ptr;
}

Value DereferenceNode::eval()
{
    return compiler->create_value(get_type(), address->eval().get());
}

const Type* DereferenceNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    auto t = assert_ptr(address->get_type())->get_element_type();
    return set_type(compiler->create_type<ReferenceType>(t, false));
}

}
