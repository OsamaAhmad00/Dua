#pragma once

#include <AST/terminals/ValueNode.h>
#include <types/StringType.h>

class StringValueNode : public ValueNode {

    bool is_nullptr;
    std::string value;
    StringType type;

    static int counter;

public:

    StringValueNode() : is_nullptr(true) {}
    StringValueNode(std::string value) : is_nullptr(false), value(std::move(value)) {}
    llvm::Constant* eval() override;
    TypeBase * get_type() override { return &type; }
};