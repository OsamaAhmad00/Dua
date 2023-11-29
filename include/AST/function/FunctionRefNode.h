#pragma once

#include "AST/values/ValueNode.h"
#include "types/FunctionType.h"

namespace dua
{

struct FunctionRefNode : public ASTNode
{
    std::string name;

    FunctionRefNode(ModuleCompiler* compiler, std::string name)
        : name(std::move(name)) { this->compiler = compiler; }

    llvm::Function* eval() override;

    TypeBase* compute_type() override;
};

}