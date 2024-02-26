#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class TeleportNode : public ASTNode
{
    ASTNode* node;

public:

    TeleportNode(ModuleCompiler* compiler, ASTNode* node) : node(node)
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        auto result = node->eval();
        result.is_teleporting = true;
        return result;
    }

    const Type* get_type() override { return node->get_type(); }
};

}
