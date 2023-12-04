#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class FunctionCallNode : public ASTNode
{
    friend class ParserAssistant;

protected:

    ASTNode* func;
    std::vector<ASTNode*> args;

public:

    FunctionCallNode(ModuleCompiler* compiler, ASTNode* func, std::vector<ASTNode*> args = {})
        : func(func), args(std::move(args)) { this->compiler = compiler; }

    llvm::CallInst* eval() override;

    Type* compute_type() override;

    ~FunctionCallNode() override;
};

}
