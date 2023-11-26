#pragma once

#include "AST/ASTNode.h"
#include <utils/ErrorReporting.h>

namespace dua
{

struct ContinueNode : public ASTNode
{
public:

    ContinueNode(ModuleCompiler* compiler) { this->compiler = compiler; }
    NoneValue eval() override {
        auto& s = continue_stack();
        if (s.empty())
            report_internal_error("Not inside a loop");
        builder().CreateBr(s.back());
        return none_value();
    }
};

}
