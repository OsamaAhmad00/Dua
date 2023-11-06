#pragma once

#include <AST/ASTNode.h>

class VariableNode : public ASTNode
{
    bool dereference;
    std::string name;

public:

    VariableNode(std::string name, bool dereference=true)
        : name(std::move(name)), dereference(dereference) {}
    llvm::Value* eval() override;
};