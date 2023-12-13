#pragma once

#include <AST/ASTNode.hpp>
#include <types/Type.hpp>

namespace dua
{

class ValueNode : public ASTNode
{
public:
    Value eval() override = 0;
};

}
