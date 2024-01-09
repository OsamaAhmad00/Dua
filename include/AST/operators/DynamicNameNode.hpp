#pragma once

#include "AST/values/ValueNode.hpp"

namespace dua
{

class DynamicNameNode : public ValueNode
{

    ASTNode* instance;
    const Type* target_type;

public:

    DynamicNameNode(ModuleCompiler* compiler, ASTNode* instance, const Type* type)
        : instance(instance), target_type(type) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}