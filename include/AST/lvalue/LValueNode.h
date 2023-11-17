#pragma once

#include "AST/ASTNode.h"

namespace dua
{

class LValueNode : public ASTNode
{

public:

    virtual TypeBase* get_element_type() = 0;
    llvm::Value* eval() override = 0;
};

}
