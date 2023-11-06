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
    NAME(ASTNode* lhs, ASTNode* rhs) : lhs(lhs), rhs(rhs) {}       \
                                                                   \
    llvm::Value* eval() override {                                 \
        return builder().OP(lhs->eval(), rhs->eval(), LABEL);      \
    }                                                              \
                                                                   \
    ~NAME() override {                                             \
        delete lhs;                                                \
        delete rhs;                                                \
    }                                                              \
};
