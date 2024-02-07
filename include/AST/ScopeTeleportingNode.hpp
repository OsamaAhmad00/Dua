#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class ScopeTeleportingNode : public ASTNode
{
    // This node is a base for nodes that teleport from
    //  one scope to another. This is to avoid the
    //  unnecessary calling of the copy constructor,
    //  then the call to the destructor. Instead, these
    //  nodes are copied bitwise, as if they were created
    //  at the target location. These nodes include
    //  function/method call nodes, if/when expression
    //  nodes, and binary operators.

public:

    // A function to call after evaluating the node,
    //  so that it has a chance to perform the work
    //  needed to complete the teleportation if any
    //  is needed
    virtual void set_teleported() { }
};

}
