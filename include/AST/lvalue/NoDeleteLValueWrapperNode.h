#pragma once

#include <AST/lvalue/LValueNode.h>
#include <AST/NoDeleteWrapperNode.h>
#include <types/TypeBase.h>

// This class is used to pass an LValueNode pointer to a temporary
//  ASTNode, which holds ownership of the pointer, and still get
//  the pointer not deleted at the owner's destruction.
class NoDeleteLValueWrapperNode : public NoDeleteWrapperNode<LValueNode>
{
public:
    NoDeleteLValueWrapperNode(LValueNode* node) : NoDeleteWrapperNode<LValueNode>(node) {}
    void set_load_value(bool b) override { ((LValueNode*)node)->set_load_value(b); }
};