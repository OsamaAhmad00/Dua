#pragma once

#include "ASTNode.hpp"
#include "lvalue/LValueNode.hpp"

namespace dua
{

class IndexingNode : public ASTNode
{
    ASTNode* lhs;
    ASTNode* rhs;

    const Type* get_element_type(ASTNode* lhs, ASTNode* rhs);

public:

    IndexingNode(ModuleCompiler* compiler, ASTNode* lhs, ASTNode* rhs)
            : lhs(lhs), rhs(rhs) { this->compiler = compiler; };

    Value eval() override;

    const Type* get_type() override;
};

}
