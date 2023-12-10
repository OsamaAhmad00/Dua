#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class DeferredActionNode : public ASTNode
{
    std::function<llvm::Value*()> func;

public:

    DeferredActionNode(ModuleCompiler* compiler, std::function<llvm::Value*()> func)
            : func(std::move(func)) { this->compiler = compiler; this->type = type; };

    llvm::Value* eval() override { return func(); };
};

}
