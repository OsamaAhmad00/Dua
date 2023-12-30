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
    if (elements.empty())
        return none_value();

    compiler->push_scope();

    for (size_t i = 0; i < elements.size() - 1; i++) {
        elements[i]->eval();
    }

    auto result =  elements.back()->eval();

    compiler->destruct_last_scope();
    compiler->pop_scope();

    return result;
}

const Type* BlockNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    return elements.back()->get_type();
}

}
