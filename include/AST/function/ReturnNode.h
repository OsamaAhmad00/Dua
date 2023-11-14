#pragma once

#include <AST/ASTNode.h>

class ReturnNode : public ASTNode
{
    ASTNode* expression;

public:

    ReturnNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }
    llvm::ReturnInst* eval() override;
    ~ReturnNode() override;
};