#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

class ForNode : public ASTNode
{
    // If a counter is not used, LLVM will assign numbers incrementally (for example for_cond1, for_body2, for_end3)
    //  which can be confusing, especially in nested expressions.
    static int _counter;

    std::vector<ASTNode*> initializations;
    ASTNode* cond_exp;
    ASTNode* body_exp;
    ASTNode* update_exp;

public:

    ForNode(ModuleCompiler* compiler, std::vector<ASTNode*> initializations,
              ASTNode* cond_exp, ASTNode* body_exp, ASTNode* update_exp)
            : initializations(std::move(initializations)), cond_exp(cond_exp),
              body_exp(body_exp), update_exp(update_exp) { this->compiler = compiler; }

    NoneValue eval() override;
};

}
