#pragma once

#include <AST/ASTNode.hpp>
#include <types/Type.hpp>

namespace dua
{

class ValueNode : public ASTNode
{
public:
    llvm::Constant* eval() override = 0;
};

}
