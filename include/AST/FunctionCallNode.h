#pragma once

#include <AST/FunctionNodeBase.h>

class FunctionCallNode : public FunctionNodeBase
{
    std::string name;
    std::vector<ASTNode*> args;

public:

    FunctionCallNode(std::string name, std::vector<ASTNode*> args)
        : name(std::move(name)), args(std::move(args)) {}
    llvm::CallInst* eval() override;
    ~FunctionCallNode() override;
};