#pragma once

#include <AST/ASTNode.hpp>
#include <resolution/ResolutionString.hpp>

namespace dua
{

class FunctionCallBase : public ASTNode
{

protected:

    std::vector<ASTNode*> args;

    FunctionCallBase(ModuleCompiler* compiler, std::vector<ASTNode*> args)
        : args(std::move(args))
    {
        this->compiler = compiler;
    }
};

}
