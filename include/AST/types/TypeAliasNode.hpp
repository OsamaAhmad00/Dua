#pragma once

#include <AST/ASTNode.hpp>
#include <types/IdentifierType.hpp>

namespace dua
{

class TypeAliasNode : public ASTNode
{
    std::string name;
    const Type* type;

public:

    TypeAliasNode(ModuleCompiler* compiler, std::string name, const Type* type)
            : name(std::move(name)), type(type) { this->compiler = compiler; };

    Value eval() override {
        auto i = type->as<IdentifierType>();
        typing_system().insert_type(name, i ? i->get_type() : type);
        return none_value();
    }
};

}
