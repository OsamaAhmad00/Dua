#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class MoveNode : public ASTNode
{
    std::string name;

public:

    MoveNode(ModuleCompiler* compiler, std::string name)
            : name(std::move(name))
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        auto result = name_resolver().symbol_table.get(name);

        result.type = result.type->get_contained_type();

        name_resolver().symbol_table.move_last_occurrence_of(name);

        result.memory_location = result.get();
        result.set(nullptr);

        // Inserting into the temp expression map before setting memory_location to nullptr
        result.id = compiler->get_temp_expr_map_unused_id();
        compiler->insert_temp_expr(result);
        result.is_teleporting = true;

        // Caching the type for further get_type calls
        set_type(result.type);

        return result;
    }

    const Type* get_type() override
    {
        // You have to take into consideration that if the
        //  type cache is cleared after calling eval(), calling
        //  this method will either cause an error (because the
        //  value is moved and is no longer in the symbol table),
        //  or return the type of another variable with the same
        //  name in an outer scope.

        if (compiler->clear_type_cache) type = nullptr;

        if (type != nullptr) return type;

        return set_type(name_resolver().symbol_table.get(name).type);
    }

    const std::string& get_name() { return name; }
};

}
