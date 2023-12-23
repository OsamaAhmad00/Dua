#pragma once

#include <AST/values/ValueNode.hpp>
#include <types/FloatTypes.hpp>

namespace dua
{

#define DEFINE_FLOAT_VALUE_NODE(TYPE, WIDTH)                                     \
class F##WIDTH##ValueNode : public ValueNode {                                   \
                                                                                 \
    TYPE value;                                                                  \
                                                                                 \
public:                                                                          \
                                                                                 \
    F##WIDTH##ValueNode(ModuleCompiler* compiler, TYPE value)                    \
        : value(value) { this->compiler = compiler; }                            \
                                                                                 \
    F##WIDTH##ValueNode(ModuleCompiler* compiler)                                \
        : F##WIDTH##ValueNode(compiler, 0.0) {}                                  \
                                                                                 \
    Value eval() override                                                        \
    {                                                                            \
        auto res = llvm::ConstantFP::get(get_type()->llvm_type(), value);        \
        return compiler->create_value(res, get_type());                          \
    }                                                                            \
                                                                                 \
    const Type* get_type() override {                                            \
        if (type == nullptr)                                                     \
            return set_type(compiler->create_type<F##WIDTH##Type>());            \
        return type;                                                             \
    }                                                                            \
};

DEFINE_FLOAT_VALUE_NODE(double, 64)

DEFINE_FLOAT_VALUE_NODE(float, 32)

}
