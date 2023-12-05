#pragma once

#include "AST/lvalue/LValueNode.hpp"

namespace dua
{

struct LoadedLValueNode : public ASTNode
{
    LValueNode* lvalue;

    LoadedLValueNode(ModuleCompiler* compiler, LValueNode* lvalue)
        : lvalue(lvalue) { this->compiler = compiler; }

    llvm::Value* eval() override;

    const Type* get_type() override;
};

}
