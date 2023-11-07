#pragma once

#include <AST/terminals/ValueNode.h>
#include <types/ArrayType.h>

class ArrayValueNode : public ValueNode
{
    // An initialized array
    std::vector<llvm::Constant*> values;
    ArrayType* type;

    llvm::Constant* eval() override { return llvm::ConstantArray::get(type->llvm_type(), values); }
    ArrayType* get_type() override { return type; }
};