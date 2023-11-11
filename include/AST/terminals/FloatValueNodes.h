#pragma once

#include <AST/terminals/ValueNode.h>
#include <types/FloatTypes.h>

#define DEFINE_FLOAT_VALUE_NODE(TYPE, WIDTH)                        \
class F##WIDTH##ValueNode : public ValueNode {                      \
    TYPE value;                                                     \
    F64Type type;                                                   \
public:                                                             \
    F##WIDTH##ValueNode(ModuleCompiler* compiler, TYPE value)       \
        : value(value), type(compiler->get_builder())               \
        { this->compiler = compiler; }                              \
    F##WIDTH##ValueNode(ModuleCompiler* compiler)                   \
        : F##WIDTH##ValueNode(compiler, 0.0) {}                     \
    llvm::Constant *eval() override                                 \
        { return llvm::ConstantFP::get(type.llvm_type(), value); }  \
    F64Type *get_type() override { return &type; }                  \
};

DEFINE_FLOAT_VALUE_NODE(double, 64)
DEFINE_FLOAT_VALUE_NODE(float, 32)