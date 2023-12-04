#include <AST/BlockNode.hpp>

namespace dua
{

BlockNode& BlockNode::append(ASTNode *element)
{
    elements.push_back(element);
    return *this;
}

llvm::Value* BlockNode::eval()
{
    compiler->push_scope();

    if (elements.empty())
        return none_value();

    for (size_t i = 0; i < elements.size() - 1; i++) {
        elements[i]->eval();
    }

    auto result =  elements.back()->eval();

    compiler->destruct_all_variables(compiler->pop_scope());

    return result;
}

Type *BlockNode::compute_type() {
    delete type;
    return type = elements.back()->get_cached_type()->clone();
}

BlockNode::~BlockNode()
{
    for (auto element: elements)
        delete element;
}

}
