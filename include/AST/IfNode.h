#pragma once

#include <AST/ASTNode.h>

class IfNode : public ASTNode
{
    // If a counter is not used, LLVM will assign numbers incrementally (for example then1, else2, condition3)
    //  which can be confusing, especially in nested expressions.
    static int _counter;

    ASTNode* cond_exp;
    ASTNode* then_exp;
    ASTNode* else_exp;

public:

    IfNode(ASTNode* cond_expr, ASTNode* then_expr, ASTNode* else_expr)
        : cond_exp(cond_expr), then_exp(then_expr), else_exp(else_expr) {}
    IfNode(ASTNode* cond_expr, ASTNode* then_expr) : IfNode(cond_expr, then_expr, nullptr) {}
    llvm::PHINode* eval() override;
    void set_else(ASTNode* node);
    IfNode& add_else_if(ASTNode* condition, ASTNode* node);
    ~IfNode() override;
};