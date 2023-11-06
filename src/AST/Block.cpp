#include <AST/Block.h>

BlockNode& BlockNode::append(ASTNode *element)
{
    elements.push_back(element);
    return *this;
}

llvm::Value* BlockNode::eval()
{
    symbol_table().push_scope();

    if (elements.empty())
        return none_value();

    for (size_t i = 0; i < elements.size() - 1; i++) {
        elements[i]->eval();
    }

    auto result =  elements.back()->eval();

    symbol_table().pop_scope();

    return result;
}

BlockNode::~BlockNode()
{
    for (auto element: elements)
        delete element;
}