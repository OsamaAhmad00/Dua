#pragma once

#include "AST/ASTNode.hpp"
#include "types/PointerType.hpp"

namespace dua
{

class MallocNode : public ASTNode
{

    std::vector<ASTNode*> args;
    ASTNode* count;

public:

    MallocNode(ModuleCompiler* compiler, const Type* type, std::vector<ASTNode*> args, ASTNode* count);

    Value eval() override;

    const Type* get_type() override;
};

}
