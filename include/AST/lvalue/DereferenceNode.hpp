#pragma once

#include <AST/lvalue/LValueNode.hpp>

namespace dua
{

class PointerType;

class DereferenceNode : public LValueNode
{
    ASTNode* address;

    const PointerType* assert_ptr(const Type* type);

public:

    DereferenceNode(ModuleCompiler* compiler, ASTNode* address)
            : address(address) { this->compiler = compiler; this->type = type; }

    Value eval() override;

    const Type* get_type() override;

    const Type* get_element_type() override;
};

}
