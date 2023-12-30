#include "AST/operators/TypeNameNode.hpp"
#include "types/PointerType.hpp"
#include "types/IntegerTypes.hpp"

namespace dua
{

Value TypeNameNode::eval()
{
    auto str = target_type->to_string();
    auto& pool = string_pool();
    auto it = pool.find(str);
    if (it != pool.end())
        return compiler->create_value(it->second, get_type());
    auto result = builder().CreateGlobalStringPtr(str, "TypeName_" + str);
    pool[str] = result;
    return compiler->create_value(result, get_type());
}

const Type *TypeNameNode::get_type() {
    if (type == nullptr)
        type = compiler->create_type<PointerType>(typing_system().create_type<I8Type>());
    return type;
}

}
