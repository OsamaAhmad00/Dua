#pragma once

#include <AST/terminals/ValueNode.h>
#include <types/StringType.h>

class StringValueNode : public ValueNode {

    bool is_nullptr;
    std::string value;
    StringType type;

    static int counter;

public:

    StringValueNode(ModuleCompiler* compiler)
        : is_nullptr(true), type(compiler->get_builder()) { this->compiler = compiler; }
    StringValueNode(ModuleCompiler* compiler, std::string value)
        : is_nullptr(false), value(std::move(value)), type(compiler->get_builder())
        { this->compiler = compiler; }
    llvm::Constant* eval() override;
    TypeBase * get_type() override { return &type; }
};