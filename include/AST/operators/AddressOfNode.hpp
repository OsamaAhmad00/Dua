#pragma once

#include <AST/ASTNode.hpp>
#include <types/PointerType.hpp>
#include <types/ReferenceType.hpp>

namespace dua
{

class AddressOfNode : public ASTNode
{

    ASTNode* node;

public:

    AddressOfNode(ModuleCompiler* compiler, ASTNode* node) : node(node)
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        auto value = node->eval();

        if (auto ref = value.type->as<ReferenceType>(); ref != nullptr) {
            if (ref->is_allocated()) {
                value.memory_location = value.get();
            }
        }

        if (value.memory_location == nullptr)
            compiler->report_error("Can't use the & operator (address-of operator) with the type " + value.type->to_string());

        value.set(value.memory_location);
        value.memory_location = nullptr;
        value.type = compiler->create_type<PointerType>(value.type);
        return value;
    }

    const Type* get_type() override
    {
        return compiler->create_type<PointerType>(node->get_type());
    }
};

}
