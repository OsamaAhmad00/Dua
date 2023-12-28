#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class IsTypeNode : public ASTNode
{
    const Type* lhs;
    const Type* rhs;

public:

    IsTypeNode(ModuleCompiler* compiler, const Type* lhs, const Type* rhs)
            : lhs(lhs), rhs(rhs)
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        bool result = *lhs == *rhs;
        return compiler->create_value(builder().getInt8(result), get_type());
    }

    const Type* get_type() override {
        if (type == nullptr) type = compiler->create_type<I8Type>();
        return type;
    }
};

}
