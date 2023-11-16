#pragma once

#include <AST/terminals/ValueNode.h>
#include <types/FloatTypes.h>

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
    llvm::Constant *eval() override                                              \
        { return llvm::ConstantFP::get(get_cached_type()->llvm_type(), value); } \
                                                                                 \
    TypeBase* compute_type() override {                                          \
        if (type == nullptr)                                                     \
            return type = compiler->create_type<F##WIDTH##Type>();               \
        return type;                                                             \
    }                                                                            \
};

DEFINE_FLOAT_VALUE_NODE(double, 64)
DEFINE_FLOAT_VALUE_NODE(float, 32)

}
