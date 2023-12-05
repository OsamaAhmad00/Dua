#pragma once

#include "AST/ASTNode.hpp"
#include "LValueNode.hpp"

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

    const Type* get_type() override;

    const Type* get_element_type() override;
};

}
