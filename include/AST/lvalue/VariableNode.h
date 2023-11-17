#pragma once

#include "LValueNode.h"

namespace dua
{

class VariableNode : public LValueNode
{
    std::string name;

public:

    VariableNode(ModuleCompiler* compiler, std::string name)
        : name(std::move(name)) { this->compiler = compiler; }
    llvm::Value* eval() override;
    TypeBase* compute_type() override;
    TypeBase* get_element_type() override;
};

}
