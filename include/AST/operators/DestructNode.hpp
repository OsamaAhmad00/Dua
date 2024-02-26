#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class DestructNode : public ASTNode
{
    ASTNode* expr;

public:

    DestructNode(ModuleCompiler* compiler, ASTNode* expr) : expr(expr)
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        auto instance = expr->eval();
        if (instance.memory_location == nullptr)
            compiler->report_error("Cannot destruct the expression with type "
                                   + instance.type->to_string() + " that has no memory address");
        instance.set(instance.memory_location);
        instance.memory_location = nullptr;
        instance.type = instance.type->get_contained_type();

        name_resolver().call_destructor(instance);

        return none_value();
    }
};

}
