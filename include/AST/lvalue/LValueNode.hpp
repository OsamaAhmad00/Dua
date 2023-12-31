#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

class LValueNode : public ASTNode
{

public:

    virtual const Type* get_element_type() = 0;

    Value eval() override = 0;
};

}
