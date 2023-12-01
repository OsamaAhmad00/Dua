#pragma once

#include <AST/ASTNode.h>

namespace dua
{

class DeferredActionNode : public ASTNode
{
    std::function<llvm::Value*()> func;

public:

    DeferredActionNode(ModuleCompiler* compiler, std::function<llvm::Value*()> func, Type* type)
            : func(func) { this->compiler = compiler; this->type = type; };
    llvm::Value* eval() override { return func(); };
    Type* compute_type() override { return type; }
};

}
