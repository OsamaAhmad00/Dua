#pragma once

#include <AST/ASTNode.h>

class BlockNode : public ASTNode
{
    std::vector<ASTNode*> elements;

public:

    BlockNode(ModuleCompiler* compiler) { this->compiler = compiler; };
    BlockNode& append(ASTNode* element);
    llvm::Value* eval() override;
    ~BlockNode() override;
};