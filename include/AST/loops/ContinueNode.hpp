#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

struct ContinueNode : public ASTNode
{
public:

    ContinueNode(ModuleCompiler* compiler) { this->compiler = compiler; }

    NoneValue eval() override {
        auto& s = continue_stack();
        if (s.empty())
            compiler->report_internal_error("Not inside a loop");
        compiler->destruct_last_scope();
        builder().CreateBr(s.back());
        return none_value();
    }
};

}
