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
    ~NAME() override {                                             \
        delete lhs;                                                \
        delete rhs;                                                \
    }                                                              \
};
