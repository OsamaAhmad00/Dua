#include "AST/lvalue/DereferenceNode.h"
#include "types/PointerType.h"
#include <utils/ErrorReporting.h>

namespace dua
{

PointerType* assert_ptr(TypeBase* type) {
    auto ptr = dynamic_cast<PointerType*>(type);
    if (!ptr) report_error("Can't dereference a non-pointer type expression");
    return ptr;
}

llvm::Value* DereferenceNode::eval()
{
    assert_ptr(address->get_cached_type());
    return address->eval();
}

TypeBase *DereferenceNode::compute_type()
{
    delete type;
    return type = assert_ptr(address->get_cached_type()->clone());
}

TypeBase* DereferenceNode::get_element_type()
{
    auto ptr = assert_ptr(get_cached_type());
    return ptr->get_element_type();
}

}
