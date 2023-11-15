#pragma once

#include <AST/ASTNode.h>

#define BINARY_EXP_NODE(NAME, OP, LABEL)                           \
class NAME : public ASTNode                                        \
{                                                                  \
    ASTNode* lhs;                                                  \
    ASTNode* rhs;                                                  \
                                                                   \
public:                                                            \
                                                                   \
    NAME(ModuleCompiler* compiler, ASTNode* lhs, ASTNode* rhs)     \
        : lhs(lhs), rhs(rhs) { this->compiler = compiler; }        \
                                                                   \
    llvm::Value* eval() override {                                 \
        return builder().OP(lhs->eval(), rhs->eval(), LABEL);      \
    }                                                              \
                                                                   \
    TypeBase* compute_type() override {                            \
        delete type;                                               \
        auto ltype = lhs->get_cached_type();                       \
        auto rtype = rhs->get_cached_type();                       \
        return type = compiler->get_winning_type(ltype, rtype);    \
    }                                                              \
                                                                   \
    ~NAME() override {                                             \
        delete lhs;                                                \
        delete rhs;                                                \
    }                                                              \
};

#define SAME_TYPE_BINARY_EXP_NODE(NAME, OP, LABEL)                                  \
class NAME : public ASTNode                                                         \
{                                                                                   \
    ASTNode* lhs;                                                                   \
    ASTNode* rhs;                                                                   \
                                                                                    \
public:                                                                             \
                                                                                    \
    NAME(ModuleCompiler* compiler, ASTNode* lhs, ASTNode* rhs)                      \
        : lhs(lhs), rhs(rhs) { this->compiler = compiler; }                         \
                                                                                    \
    llvm::Value* eval() override {                                                  \
        auto lhs_value = lhs->eval();                                               \
        auto rhs_value = rhs->eval();                                               \
        rhs_value = compiler->cast_value(rhs_value, lhs_value->getType());          \
        if (rhs_value == nullptr)                                                   \
            throw std::runtime_error("Type mismatch between the two operands");     \
        return builder().OP(lhs_value, rhs_value, LABEL);                           \
    }                                                                               \
                                                                                    \
    TypeBase* compute_type() override {                                             \
        delete type;                                                                \
        auto ltype = lhs->get_cached_type();                                        \
        auto rtype = rhs->get_cached_type();                                        \
        return type = compiler->get_winning_type(ltype, rtype);                     \
    }                                                                               \
                                                                                    \
    ~NAME() override {                                                              \
        delete lhs;                                                                 \
        delete rhs;                                                                 \
    }                                                                               \
};

