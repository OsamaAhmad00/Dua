#pragma once

#include <AST/ASTNode.h>
#include <types/TypeBase.h>

// This class is used to pass an ASTNode pointer to a temporary
//  ASTNode, which holds ownership of the pointer, and still get
//  the pointer not deleted at the owner's destruction.
template <typename T>
class NoDeleteWrapperNode : public T
{
protected:

    ASTNode* node;

public:

    NoDeleteWrapperNode(ASTNode* node) : node(node) {}
    llvm::Value * eval() override { return node->eval(); };
    TypeBase* compute_type() override { return this->type = node->compute_type(); };
    ~NoDeleteWrapperNode() override = default;
};