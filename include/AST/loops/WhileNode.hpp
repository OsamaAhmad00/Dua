#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

class WhileNode : public ASTNode
{
    // If a counter is not used, LLVM will assign numbers incrementally (for example while_cond1, while_body2, while_end3)
    //  which can be confusing, especially in nested expressions.
    static int _counter;

    ASTNode* cond_exp;
    ASTNode* body_exp;

public:

    WhileNode(ModuleCompiler* compiler, ASTNode* cond_exp, ASTNode* body_exp)
        : cond_exp(cond_exp), body_exp(body_exp) { this->compiler = compiler; }

    NoneValue eval() override;
};

}
