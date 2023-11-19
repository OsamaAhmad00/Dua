#include <AST/values/StringValueNode.h>

namespace dua
{

int StringValueNode::counter = 0;

llvm::Constant* StringValueNode::eval()
{
    if (is_nullptr) return type->default_value();
    auto& pool = compiler->get_string_pool();
    auto it = pool.find(value);
    if (it != pool.end())
        return it->second;
    auto result = builder().CreateGlobalStringPtr(value, "StringLiteral" + std::to_string(counter++));
    pool[value] = result;
    return result;
}

TypeBase *StringValueNode::compute_type() {
    if (type == nullptr) return type = compiler->create_type<StringType>();
    return type;
}

}
