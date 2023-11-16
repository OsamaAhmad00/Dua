#pragma once

#include <AST/ASTNode.h>
#include <FunctionInfo.h>

namespace dua
{

class FunctionCallNode : public ASTNode
{
    std::string name;
    std::vector<ASTNode*> args;

public:

    FunctionCallNode(ModuleCompiler* compiler, std::string name, std::vector<ASTNode*> args)
        : name(std::move(name)), args(std::move(args)) { this->compiler = compiler; }

    llvm::CallInst* eval() override;

    TypeBase* compute_type() override;

    ~FunctionCallNode() override;
};

}
