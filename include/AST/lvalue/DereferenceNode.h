#pragma once

#include <AST/lvalue/LValueNode.h>

namespace dua
{

class DereferenceNode : public LValueNode
{
    ASTNode* address;

public:

    DereferenceNode(ModuleCompiler* compiler, ASTNode* address)
            : address(address) { this->compiler = compiler; this->type = type; }
    llvm::Value* eval() override;
    Type* compute_type() override;
    Type* get_element_type() override;
    ~DereferenceNode() override { delete address; }
};

}
