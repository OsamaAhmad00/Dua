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
        auto ptr = dynamic_cast<const PointerType*>(expr->get_type());
        if (ptr == nullptr)
            report_error("The delete operator only accepts a pointer type, not a "
                + expr->get_type()->to_string());

        auto value = compiler->create_value(expr->eval(), ptr->get_element_type());
        name_resolver().call_destructor(value);

        // Here, we pass the param type of free, regardless of the actual type of the expression, to
        // avoid the typing system complaining about the free function having the wrong pointer type
        auto param_type = name_resolver().get_function_no_overloading("free").type->param_types.front();
        value.type = param_type;
        name_resolver().call_function("free", { value });

        return none_value();
    }
};

}
