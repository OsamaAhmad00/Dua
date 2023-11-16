#pragma once

#include <AST/ASTNode.h>
#include <types/TypeBase.h>

namespace dua
{

class ValueNode : public ASTNode
{
public:
    llvm::Constant* eval() override = 0;
};

}
