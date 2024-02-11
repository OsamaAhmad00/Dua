#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class MoveNode : public ASTNode
{
    std::string name;
    bool call_destructor;

public:

    MoveNode(ModuleCompiler* compiler, std::string name, bool call_destructor = false)
            : name(std::move(name)), call_destructor(call_destructor)
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        auto result = name_resolver().symbol_table.get(name);
        name_resolver().symbol_table.move_last_occurrence_of(name);
        if (call_destructor)
            name_resolver().call_destructor(result);
        result.memory_location = result.get();
        result.set(nullptr);
        result.get();  // Just loading the value
        result.memory_location = nullptr;  // So that it can't be bound as a reference
        result.is_teleporting = true;
        return result;
    }

    const Type* get_type() override {
        return name_resolver().symbol_table.get(name).type;
    }
};

}
