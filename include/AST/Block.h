#pragma once

#include <AST/ASTNode.h>

class BlockNode : public ASTNode
{
    std::vector<ASTNode*> elements;

public:

    BlockNode() {};
    BlockNode& append(ASTNode* element);
    llvm::Value* eval() override;
    ~BlockNode() override;
};