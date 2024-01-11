#pragma once

#include <AST/lvalue/LValueNode.hpp>

namespace dua
{

class DynamicCastNode : public ASTNode
{
    ASTNode* instance;
    const Type* target_type;

public:

    DynamicCastNode(ModuleCompiler* compiler, ASTNode* instance, const Type* target_type)
        : instance(instance), target_type(target_type) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}