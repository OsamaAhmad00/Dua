#include <AST/BlockNode.hpp>

namespace dua
{

BlockNode& BlockNode::append(ASTNode *element)
{
    elements.push_back(element);
    return *this;
}

Value BlockNode::eval()
{
    name_resolver().push_scope();

    if (elements.empty())
        return none_value();

    for (size_t i = 0; i < elements.size() - 1; i++) {
        elements[i]->eval();
    }

    auto result =  elements.back()->eval();

    name_resolver().destruct_all_variables(name_resolver().pop_scope());

    return result;
}

const Type* BlockNode::get_type() {
    return elements.back()->get_type();
}

}
