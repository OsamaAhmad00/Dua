#pragma once

#include "../ASTNode.hpp"
#include "../../Value.hpp"

namespace dua
{

class LLVMValueNode : public ASTNode
{
    llvm::Value* value;

public:

    LLVMValueNode(ModuleCompiler* compiler, const Value& value)
            : value(value.ptr) { this->compiler = compiler; this->type = value.type; };

    llvm::Value* eval() override { return value; };
};

}
