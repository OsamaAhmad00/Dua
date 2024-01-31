#pragma once

#include <AST/ScopeTeleportingNode.hpp>

namespace dua
{

class BlockNode : public ScopeTeleportingNode
{
    std::vector<ASTNode*> elements;

public:

    BlockNode(ModuleCompiler* compiler, std::vector<ASTNode*> elements)
        : elements(std::move(elements)) { this->compiler = compiler; };

    BlockNode& append(ASTNode* element);

    Value eval() override;

    const Type* get_type() override;
};

}
