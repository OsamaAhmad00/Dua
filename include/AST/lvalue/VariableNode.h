#pragma once

#include "LValueNode.h"

class VariableNode : public LValueNode
{
    std::string name;

public:

    VariableNode(ModuleCompiler* compiler, std::string name, bool get_address=false)
        : name(std::move(name)) { this->compiler = compiler; this->load_value = !get_address; }
    llvm::Value* eval() override;
    TypeBase* compute_type() override;
};