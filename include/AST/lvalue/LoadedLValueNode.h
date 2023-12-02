#pragma once

#include "AST/lvalue/LValueNode.h"

namespace dua
{

struct LoadedLValueNode : public ASTNode
{
    LValueNode* lvalue;

    LoadedLValueNode(ModuleCompiler* compiler, LValueNode* lvalue)
        : lvalue(lvalue) { this->compiler = compiler; }

    llvm::Value* eval() override;

    Type* compute_type() override;

    ~LoadedLValueNode() override { delete lvalue; }
};

}
