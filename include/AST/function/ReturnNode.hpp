#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class ReturnNode : public ASTNode
{
    ASTNode* expression;

public:

    ReturnNode(ModuleCompiler* compiler, ASTNode* expression)
        : expression(expression) { this->compiler = compiler; }

    Value eval() override;
};

}
