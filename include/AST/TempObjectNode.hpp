#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

class TempObjectNode : public ASTNode
{
    const Type* type;
    std::vector<ASTNode*> args;

public:

    TempObjectNode(ModuleCompiler* compiler, const Type* type, std::vector<ASTNode*> args)
            : type(type), args(std::move(args)) { this->compiler = compiler; };

    Value eval() override;

    const Type* get_type() override;
};

}
