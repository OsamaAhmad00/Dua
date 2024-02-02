#pragma once

#include <AST/values/ValueNode.hpp>

namespace dua
{

class StringValueNode : public ValueNode
{

    std::string value;

    static int counter;

public:

    StringValueNode(ModuleCompiler* compiler, std::string value)
        : value(std::move(value))
        { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}