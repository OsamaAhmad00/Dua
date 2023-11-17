#pragma once

#include "AST/lvalue/LValueNode.h"

namespace dua
{

class LoadedLValueNode : public ASTNode
{

    LValueNode* lvalue;

public:

    LoadedLValueNode(ModuleCompiler* compiler, LValueNode* lvalue)
        : lvalue(lvalue) { this->compiler = compiler; }
    llvm::Value* eval() override;
    TypeBase* compute_type() override;
    ~LoadedLValueNode() override { delete lvalue; }
};

}
