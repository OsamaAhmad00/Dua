#pragma once

#include <AST/terminals/ValueNode.h>
#include <types/ArrayType.h>

class ArrayValueNode : public ValueNode
{
    // An initialized array
    std::vector<llvm::Constant*> values;
    ArrayType* type;

    ArrayValueNode(ModuleCompiler* compiler, std::vector<llvm::Constant*> values)
        : values(std::move(values)) { this->compiler = compiler; }

    llvm::Constant* eval() override { return llvm::ConstantArray::get(type->llvm_type(), values); }
    ArrayType* get_type() override { return type; }
};