#pragma once

#include <AST/values/ValueNode.h>
#include <types/StringType.h>

namespace dua
{

class StringValueNode : public ValueNode {

    bool is_nullptr;
    std::string value;

    static int counter;

public:

    StringValueNode(ModuleCompiler* compiler)
        : is_nullptr(true) { this->compiler = compiler; }
    StringValueNode(ModuleCompiler* compiler, std::string value)
        : is_nullptr(false), value(std::move(value))
        { this->compiler = compiler; }
    llvm::Constant* eval() override;
    TypeBase* compute_type() override;
};

}