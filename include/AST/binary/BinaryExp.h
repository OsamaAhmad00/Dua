#pragma once

#include <AST/ASTNode.h>

namespace dua
{

#define SAME_TYPE_BINARY_EXP_NODE(NAME, OP, LABEL)                                    \
class NAME : public ASTNode                                                           \
{                                                                                     \
    ASTNode* lhs;                                                                     \
    ASTNode* rhs;                                                                     \
                                                                                      \
public:                                                                               \
                                                                                      \
    NAME(ModuleCompiler* compiler, ASTNode* lhs, ASTNode* rhs)                        \
        : lhs(lhs), rhs(rhs) { this->compiler = compiler; }                           \
                                                                                      \
    static llvm::Value* perform(ModuleCompiler* compiler,                             \
        llvm::Value* lhs, llvm::Value* rhs, llvm::Type* type) {                       \
        lhs = compiler->cast_value(lhs, type);                                        \
        rhs = compiler->cast_value(rhs, type);                                        \
        if (lhs == nullptr || rhs == nullptr)                                         \
            throw std::runtime_error("Type mismatch between the two operands");       \
        return compiler->get_builder()->OP(lhs, rhs, LABEL);                          \
    }                                                                                 \
                                                                                      \
    llvm::Value* eval() override {                                                    \
        auto lhs_value = lhs->eval();                                                 \
        auto rhs_value = rhs->eval();                                                 \
        return NAME::perform(                                                         \
            compiler,                                                                 \
            lhs_value,                                                                \
            rhs_value,                                                                \
            get_cached_type()->llvm_type()                                            \
        );                                                                            \
    }                                                                                 \
                                                                                      \
    TypeBase* compute_type() override {                                               \
        delete type;                                                                  \
        auto ltype = lhs->get_cached_type();                                          \
        auto rtype = rhs->get_cached_type();                                          \
        return type = compiler->get_winning_type(ltype, rtype)->clone();              \
    }                                                                                 \
                                                                                      \
    ~NAME() override {                                                                \
        delete lhs;                                                                   \
        delete rhs;                                                                   \
    }                                                                                 \
};

}
