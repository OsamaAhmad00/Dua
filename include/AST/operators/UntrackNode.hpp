#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class UntrackNode : public ASTNode
{
    std::string name;

public:

    UntrackNode(ModuleCompiler* compiler, std::string name) : name(std::move(name))
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        auto result = name_resolver().symbol_table.get(name);

        result.type = result.type->get_contained_type();

        name_resolver().symbol_table.move_last_occurrence_of(name);

        return none_value();
    }
};

}
