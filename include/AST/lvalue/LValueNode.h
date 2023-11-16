#pragma once

#include "AST/ASTNode.h"

namespace dua
{

class LValueNode : public ASTNode
{
protected:

    bool load_value = false;

public:

    virtual void set_load_value(bool b) { this->load_value = b; }
    llvm::Value* eval() = 0;
};

}
