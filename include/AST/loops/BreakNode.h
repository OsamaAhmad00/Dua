#pragma once

#include "AST/ASTNode.h"

struct BreakNode : public ASTNode
{
public:

    BreakNode(ModuleCompiler* compiler) { this->compiler = compiler; }
    NoneValue eval() override {
        auto& s = compiler->get_break_stack();
        if (s.empty())
            throw std::runtime_error("Not inside a loop");
        builder().CreateBr(s.back());
        return none_value();
    }
};