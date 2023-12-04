#pragma once

#include "LValueNode.hpp"

namespace dua
{

class VariableNode : public LValueNode
{

public:

    std::string name;

    VariableNode(ModuleCompiler* compiler, std::string name)
        : name(std::move(name)) { this->compiler = compiler; }

    llvm::Value* eval() override;

    Type* compute_type() override;

    Type* get_element_type() override;

    virtual bool is_function() const;
};

}
