#pragma once

#include <AST/values/ValueNode.hpp>
#include <types/ArrayType.hpp>

namespace dua
{

class ArrayValueNode : public ValueNode
{
    // An initialized array
    std::vector<llvm::Constant*> values;
    Type* element_type;
    size_t size;

public:

    ArrayValueNode(ModuleCompiler* compiler, size_t size, Type* element_type, std::vector<llvm::Constant*> values)
        : values(std::move(values)), size(size), element_type(element_type) { this->compiler = compiler; }

    llvm::Constant* eval() override {
        return llvm::ConstantArray::get((llvm::ArrayType*)(get_type()->llvm_type()), values);
    }

    const Type* get_type() override {
        if (type == nullptr) return type = compiler->create_type<ArrayType>(element_type, size);
        return type;
    }
};

}
