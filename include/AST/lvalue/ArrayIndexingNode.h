#pragma once

#include "AST/ASTNode.h"
#include "LValueNode.h"

namespace dua
{

class ArrayIndexingNode : public LValueNode
{
    LValueNode* lvalue;
    ASTNode* index;

public:

    ArrayIndexingNode(ModuleCompiler* compiler, LValueNode* lvalue, ASTNode* index)
            : lvalue(lvalue), index(index) { this->compiler = compiler; };
    llvm::Value* eval() override;
    Type* compute_type() override;
    Type* get_element_type() override;
    ~ArrayIndexingNode() override;
};

}
