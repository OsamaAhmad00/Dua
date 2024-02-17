#include <AST/ASTNode.hpp>
#include "types/ReferenceType.hpp"
#include "types/VoidType.hpp"

namespace dua
{

std::vector<Value> ASTNode::eval_args(const std::vector<ASTNode*>& args)
{
    auto n = args.size();
    std::vector<Value> evaluated(n);
    for (size_t i = 0; i < n; i++)
    {
        // nullptr args are placeholders that
        //  will be substituted later
        if (args[i] != nullptr)
            evaluated[i] = args[i]->eval();
    }

    return evaluated;
}

NoneValue ASTNode::none_value() { return Value { nullptr, nullptr, nullptr, nullptr }; }

const Type* ASTNode::get_type()
{
    if (type == nullptr)
        return set_type(compiler->create_type<VoidType>());
    return type;
}

const Type* ASTNode::set_type(const Type* type) {
    if (!compiler->stop_caching_types)
        this->type = type;
    return type;
}

}
