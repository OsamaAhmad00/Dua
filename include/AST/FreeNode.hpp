#pragma once

#include <AST/ASTNode.hpp>
#include <types/VoidType.hpp>
#include "types/PointerType.hpp"

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
        auto ptr = dynamic_cast<PointerType*>(expr->get_cached_type());
        if (ptr == nullptr)
            report_error("The delete operator only accepts a pointer type, not a "
                + expr->get_cached_type()->to_string());

        auto value = expr->eval();
        name_resolver().call_destructor({ value, ptr->get_element_type() });
        name_resolver().call_function("free", { value });

        return none_value();
    }

    Type* compute_type() override { return type; };

    ~FreeNode() override { delete expr; }
};

}
