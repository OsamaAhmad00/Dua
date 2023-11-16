#pragma once

#include "LValueNode.h"

namespace dua
{

class AddressNode : public LValueNode
{
    ASTNode* address;

public:

    AddressNode(ModuleCompiler* compiler, ASTNode* address, bool load_value=false)
            : address(address) { this->compiler = compiler; this->load_value = load_value; }
    llvm::Value* eval() override;
    TypeBase* compute_type() override;
    ~AddressNode() override { delete address; }
};

}
