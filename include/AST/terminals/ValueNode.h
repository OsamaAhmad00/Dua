#pragma once

#include <AST/ASTNode.h>

class ValueNode : public ASTNode
{
public:
    virtual llvm::Constant* eval() override = 0;
    virtual llvm::Constant* default_value() = 0;
    virtual llvm::Type*  llvm_type() = 0;
};