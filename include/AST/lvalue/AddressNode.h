#pragma once

#include "LValueNode.h"

class AddressNode : public LValueNode
{
    bool load_value;
    ASTNode* address;

public:

    AddressNode(ModuleCompiler* compiler, ASTNode* address, bool load_value=false)
            : address(address), load_value(load_value) { this->compiler = compiler; }
    llvm::Value* eval() override;
    TypeBase* compute_type() override;
    ~AddressNode() override { delete address; }
};