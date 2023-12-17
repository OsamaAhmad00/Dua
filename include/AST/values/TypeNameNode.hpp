#pragma once

#include <AST/values/ValueNode.hpp>

namespace dua
{

class TypeNameNode : public ValueNode {

    const Type* target_type;

public:

    TypeNameNode(ModuleCompiler* compiler, const Type* type)
            : target_type(type) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}