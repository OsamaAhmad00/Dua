#include <AST/values/StringValueNode.hpp>
#include "types/PointerType.hpp"
#include "types/IntegerTypes.hpp"

namespace dua
{

int StringValueNode::counter = 0;

Value StringValueNode::eval()
{
    if (is_nullptr) return type->default_value();
    auto& pool = string_pool();
    auto it = pool.find(value);
    if (it != pool.end())
        return compiler->create_value(it->second, get_type());
    auto result = builder().CreateGlobalStringPtr(value, "StringLiteral" + std::to_string(counter++));
    pool[value] = result;
    return compiler->create_value(result, get_type());
}

const Type *StringValueNode::get_type() {
    if (type == nullptr)
        return type = typing_system().create_type<PointerType>(typing_system().create_type<I8Type>());
    return type;
}

}
