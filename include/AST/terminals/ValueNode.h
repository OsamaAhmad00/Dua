#pragma once

#include <AST/ASTNode.h>
#include <types/TypeBase.h>

class ValueNode : public ASTNode
{
public:
    virtual llvm::Constant* eval() override = 0;
    virtual TypeBase* get_type() = 0;
};