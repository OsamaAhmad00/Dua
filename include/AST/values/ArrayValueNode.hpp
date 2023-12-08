#pragma once

#include <AST/values/ValueNode.hpp>
#include <types/ArrayType.hpp>

namespace dua
{

class ArrayValueNode : public ValueNode
{
    std::vector<ASTNode*> values;

public:

    ArrayValueNode(ModuleCompiler* compiler, std::vector<ASTNode*> values, const Type* type)
        : values(std::move(values))
    {
        this->compiler = compiler;
        this->type = compiler->create_type<ArrayType>(type, this->values.size());
    }

    llvm::Constant* eval() override
    {
        std::vector<llvm::Constant*> evaluated(values.size());
        auto element_type = ((ArrayType*)type)->get_element_type();
        for (size_t i = 0; i < values.size(); i++) {
            if (!typing_system().is_castable(values[i]->get_type(), element_type))
                report_error("Array initializer elements must be of the same type. Can't have both " +
                             type->to_string() + " and " + values[i]->get_type()->to_string() + " types in an initializer together");
            auto value = compiler->create_value(values[i]->eval(), values[i]->get_type());
            auto casted = typing_system().cast_value(value, element_type);
            evaluated[i] = llvm::dyn_cast<llvm::Constant>(casted);
            if (evaluated[i] == nullptr)
                report_error("Array initializers can't be initialized with "
                + values[i]->get_type()->to_string() + " type, which doesn't evaluate to a constant");
        }

        return llvm::ConstantArray::get((llvm::ArrayType*)(get_type()->llvm_type()), std::move(evaluated));
    }

    const Type* get_type() override {
        return type;
    }
};

}
