#pragma once

#include <AST/ASTNode.h>
#include <types/VoidType.h>
#include "types/PointerType.h"

namespace dua
{

class FreeNode : public ASTNode
{
    ASTNode* expr;

public:

    FreeNode(ModuleCompiler* compiler, ASTNode* expr) : expr(expr)
    {
        this->compiler = compiler;
        this->type = compiler->create_type<VoidType>();
    }

    NoneValue eval() override
    {
        if (dynamic_cast<PointerType*>(expr->get_cached_type()) == nullptr)
            report_error("The delete operator only accepts a pointer type, not a "
                + expr->get_cached_type()->to_string());

        compiler->call_function("free", { expr->eval() });

        return none_value();
    }

    Type* compute_type() override { return type; };

    ~FreeNode() override { delete expr; }
};

}
