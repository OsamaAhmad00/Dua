#pragma once

#include <AST/ASTNode.h>

namespace dua
{

class FunctionDefinitionNode : public ASTNode
{
    std::string name;
    ASTNode* body;  // Can be nullptr in case of declaration

    llvm::Function* define_function();

public:

    FunctionDefinitionNode(ModuleCompiler* compiler, std::string name, ASTNode* body);

    void set_body(ASTNode* body) { assert(this->body == nullptr); this->body = body; }

    llvm::Function* eval() override;

    ~FunctionDefinitionNode() override;
};

}
