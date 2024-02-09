#pragma once

#include "AST/ASTNode.hpp"
#include "types/PointerType.hpp"

namespace dua
{

class MallocNode : public ASTNode
{

    std::vector<ASTNode*> args;
    ASTNode* count;
    bool is_array;
    bool call_constructors;

public:

    MallocNode(ModuleCompiler* compiler, const Type* type, std::vector<ASTNode*> args, ASTNode* count, bool is_array = false, bool call_constructors = true);

    Value eval() override;

    const Type* get_type() override;
};

}
