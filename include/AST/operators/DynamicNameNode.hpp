#pragma once

#include "AST/values/ValueNode.hpp"

namespace dua
{

class DynamicNameNode : public ValueNode {

    ASTNode* id;

public:

    DynamicNameNode(ModuleCompiler* compiler, ASTNode* id)
            : id(id) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}