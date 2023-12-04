#pragma once

#include <AST/values/ValueNode.hpp>
#include <types/IntegerTypes.hpp>

namespace dua
{

#define DEFINE_INTEGER_VALUE_NODE(WIDTH)                                 \
class I##WIDTH##ValueNode : public ValueNode                             \
{                                                                        \
    int##WIDTH##_t value;                                                \
                                                                         \
public:                                                                  \
                                                                         \
    I##WIDTH##ValueNode(ModuleCompiler* compiler, int##WIDTH##_t value)  \
        : value(value) { this->compiler = compiler; }                    \
                                                                         \
    I##WIDTH##ValueNode(ModuleCompiler* compiler)                        \
        : I##WIDTH##ValueNode(compiler, 0) {}                            \
                                                                         \
    llvm::Constant* eval() override {                                    \
        return builder().getInt##WIDTH(value);                           \
    }                                                                    \
                                                                         \
    Type* compute_type() override {                                  \
        if (type == nullptr)                                             \
            return type = compiler->create_type<I##WIDTH##Type>();       \
        return type;                                                     \
    }                                                                    \
};

DEFINE_INTEGER_VALUE_NODE(64)
DEFINE_INTEGER_VALUE_NODE(32)
DEFINE_INTEGER_VALUE_NODE(16)
DEFINE_INTEGER_VALUE_NODE(8)

}
