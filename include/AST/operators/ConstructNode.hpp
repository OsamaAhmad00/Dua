#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class ConstructNode : public ASTNode
{
    ASTNode* expr;
    std::vector<ASTNode*> args;

public:

    ConstructNode(ModuleCompiler* compiler, ASTNode* expr, std::vector<ASTNode*> args) : expr(expr), args(std::move(args))
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        auto instance = expr->eval();
        if (instance.memory_location == nullptr)
            compiler->report_error("Cannot construct the expression with type "
                + instance.type->to_string() + " that has no memory address");
        instance.set(instance.memory_location);
        instance.memory_location = nullptr;
        instance.type = instance.type->get_contained_type();

        std::vector<Value> evaluated_args(args.size());
        for (int i = 0; i < args.size(); i++)
            evaluated_args[i] = args[i]->eval();

        name_resolver().call_constructor(instance, std::move(evaluated_args));

        return none_value();
    }
};

}
