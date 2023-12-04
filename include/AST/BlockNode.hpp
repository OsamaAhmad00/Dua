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
    llvm::Value* eval() override;
    Type* compute_type() override;
    ~BlockNode() override;
};

}
