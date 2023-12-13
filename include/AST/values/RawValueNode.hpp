#pragma once

#include "../ASTNode.hpp"
#include "../../Value.hpp"

namespace dua
{

class RawValueNode : public ASTNode
{
    Value value;

public:

    RawValueNode(ModuleCompiler* compiler, const Value& value)
            : value(value) { this->compiler = compiler; this->type = value.type; };

    Value eval() override { return value; };
};

}
