#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class DeferredActionNode : public ASTNode
{
    std::function<Value()> func;

public:

    DeferredActionNode(ModuleCompiler* compiler, std::function<Value()> func)
            : func(std::move(func)) { this->compiler = compiler; this->type = type; };

    Value eval() override { return func(); };
};

}
