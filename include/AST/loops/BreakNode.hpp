#pragma once

#include "AST/ASTNode.hpp"
#include <utils/ErrorReporting.hpp>

namespace dua
{

struct BreakNode : public ASTNode
{
public:

    BreakNode(ModuleCompiler* compiler) { this->compiler = compiler; }

    NoneValue eval() override {
        auto& s = break_stack();
        if (s.empty())
            report_internal_error("Not inside a loop");
        builder().CreateBr(s.back());
        return none_value();
    }
};

}
