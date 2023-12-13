#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class BlockNode : public ASTNode
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
