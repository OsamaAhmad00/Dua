#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class StaticValueNode : public ASTNode
{
    Value result;

public:

    StaticValueNode(ModuleCompiler* compiler, const Value& result)
            : result(result) { this->compiler = compiler; };

    Value eval() override { return result; };

    const Type* get_type() override { return result.type; }
};

}
