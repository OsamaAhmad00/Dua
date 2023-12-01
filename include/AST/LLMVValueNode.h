#pragma once

#include <AST/ASTNode.h>

namespace dua
{

class LLVMValueNode : public ASTNode
{
    llvm::Value* value;

public:

    LLVMValueNode(ModuleCompiler* compiler, llvm::Value* value, Type* type)
            : value(value) { this->compiler = compiler; this->type = type; };
    NoneValue eval() override { return value; };
    Type* compute_type() override { return type; }
};

}
