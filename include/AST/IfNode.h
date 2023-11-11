#pragma once

#include <AST/ASTNode.h>

class IfNode : public ASTNode
{
    // If a counter is not used, LLVM will assign numbers incrementally (for example then1, else2, condition3)
    //  which can be confusing, especially in nested expressions.
    static int _counter;

    ASTNode* cond_expr;
    ASTNode* then_expr;
    ASTNode* else_expr;

public:

    IfNode(ModuleCompiler* compiler) : IfNode(compiler, nullptr, nullptr, nullptr) {}
    IfNode(ModuleCompiler* compiler, ASTNode* cond_expr, ASTNode* then_expr, ASTNode* else_expr)
        : cond_expr(cond_expr), then_expr(then_expr), else_expr(else_expr) { this->compiler = compiler; }
    llvm::PHINode* eval() override;
    // These two functions return the next insertion point.
    // An if, else-if, else conditional is represented as a nested IfNode, which
    //  contains only then and else branches. For inserting a branch at the
    //  top (evaluated first), you need to add it from the original IfNode.
    //  To add a branch at the bottom (evaluated last), you need to insert
    //  it from the deepest node in the IfNode chain. This means that you can't
    //  call both functions on the results returned from each other.
    // Make sure to invoke the correct function at the correct node. Otherwise,
    //  aside from the incorrect behaviour, you might get bugs that are hard
    //  to debug, such as a double-free bug (two or more nodes pointing to the
    //  same else IfNode), null-dereferencing (a node pointing to the same else
    //  IfNode as another node, is created and destroyed and deleted its else
    //  node, then the other node tried to access it), and other possible bugs.
    IfNode& add_branch_at_top(ASTNode* condition, ASTNode* node);
    IfNode& add_branch_at_bottom(ASTNode* condition, ASTNode* node);
    void set_else(ASTNode* node);
    ~IfNode() override;
};