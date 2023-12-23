#include <AST/lvalue/LoadedLValueNode.hpp>
#include <types/PointerType.hpp>
#include "types/ArrayType.hpp"

namespace dua
{

Value LoadedLValueNode::eval()
{
    auto memory_location = lvalue->eval();

    // If it's pointing to a function, don't load the address
    auto element_type = lvalue->get_element_type();
    if (dynamic_cast<const FunctionType*>(element_type) != nullptr)
        return memory_location;

    return compiler->create_value(get_type(), memory_location.get());
}

const Type *LoadedLValueNode::get_type()
{
    if (type != nullptr) return type;
    auto result = lvalue->get_type();
    const Type* ptr = result->as<PointerType>();
    if (ptr == nullptr) {
        ptr = result->as<ArrayType>();
        assert(ptr != nullptr);
    }

    // If it's pointing to a function, don't load the address
    auto element_type = lvalue->get_element_type();
    if (dynamic_cast<const FunctionType*>(element_type) != nullptr)
        return set_type(ptr);

    return set_type(lvalue->get_element_type());
}

}
