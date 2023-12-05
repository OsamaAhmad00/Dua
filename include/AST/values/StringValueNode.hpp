#pragma once

#include <AST/values/ValueNode.hpp>
#include <types/StringType.hpp>

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
    Type* compute_type() override;
};

}