#pragma once

#include <AST/terminals/ValueNode.h>
#include <types/IntegerTypes.h>

#define DEFINE_INTEGER_VALUE_NODE(WIDTH)                                 \
class I##WIDTH##ValueNode : public ValueNode                             \
{                                                                        \
    int##WIDTH##_t value;                                                \
    I##WIDTH##Type type;                                                 \
                                                                         \
public:                                                                  \
                                                                         \
    I##WIDTH##ValueNode(ModuleCompiler* compiler, int##WIDTH##_t value)  \
        : value(value), type(compiler->get_builder())                    \
        { this->compiler = compiler; }                                   \
                                                                         \
    I##WIDTH##ValueNode(ModuleCompiler* compiler)                        \
        : I##WIDTH##ValueNode(compiler, 0) {}                            \
                                                                         \
    llvm::Constant* eval() override {                                    \
        return builder().getInt##WIDTH(value);                           \
    }                                                                    \
                                                                         \
    I##WIDTH##Type* get_type() override { return &type; }                \
};

DEFINE_INTEGER_VALUE_NODE(64)
DEFINE_INTEGER_VALUE_NODE(32)
DEFINE_INTEGER_VALUE_NODE(16)
DEFINE_INTEGER_VALUE_NODE(8)
