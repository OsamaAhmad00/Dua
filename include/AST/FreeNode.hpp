#pragma once

#include <AST/ASTNode.hpp>
#include <types/VoidType.hpp>
#include "types/PointerType.hpp"

namespace dua
{

class FreeNode : public ASTNode
{
    ASTNode* expr;
    bool is_array;
    bool call_destructors;

public:

    FreeNode(ModuleCompiler* compiler, ASTNode* expr, bool is_array = false, bool call_destructors = true);

    NoneValue eval() override;
};

}
