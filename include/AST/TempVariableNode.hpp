#pragma once

#include "AST/ScopeTeleportingNode.hpp"

namespace dua
{

class TempVariableNode : public ScopeTeleportingNode
{
    const Type* type;
    std::vector<ASTNode*> args;

    static size_t counter;

public:

    std::string name;

    TempVariableNode(ModuleCompiler* compiler, const Type* type, std::vector<ASTNode*> args)
            : type(type), args(std::move(args)) { this->compiler = compiler; };

    Value eval() override;

    const Type* get_type() override;

    void set_teleported() override;
};

}
