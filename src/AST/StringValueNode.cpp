#include <AST/values/StringValueNode.hpp>
#include "types/PointerType.hpp"
#include "types/IntegerTypes.hpp"

namespace dua
{

int StringValueNode::counter = 0;

llvm::Constant* StringValueNode::eval()
{
    if (is_nullptr) return type->default_value();
    auto& pool = string_pool();
    auto it = pool.find(value);
    if (it != pool.end())
        return it->second;
    auto result = builder().CreateGlobalStringPtr(value, "StringLiteral" + std::to_string(counter++));
    pool[value] = result;
    return result;
}

const Type *StringValueNode::get_type() {
    if (type == nullptr)
        return type = typing_system().create_type<PointerType>(typing_system().create_type<I8Type>());
    return type;
}

}
