#pragma once

#include <AST/ASTNode.h>

class VariableNode : public ASTNode
{
    bool get_address;
    std::string name;

public:

    VariableNode(std::string name, bool get_address=false)
        : name(std::move(name)), get_address(get_address) {}
    llvm::Value* eval() override;
};