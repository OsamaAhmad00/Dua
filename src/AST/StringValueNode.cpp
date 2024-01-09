#include <AST/values/StringValueNode.hpp>
#include "types/PointerType.hpp"
#include "types/IntegerTypes.hpp"

namespace dua
{

int StringValueNode::counter = 0;

Value StringValueNode::eval()
{
    if (is_nullptr) return type->default_value();
    return compiler->create_string("StringLiteral" + std::to_string(counter++), value);
}

const Type *StringValueNode::get_type() {
    if (type == nullptr)
        return set_type(typing_system().create_type<PointerType>(typing_system().create_type<I8Type>()));
    return type;
}

}
