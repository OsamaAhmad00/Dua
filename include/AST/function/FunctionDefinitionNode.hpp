#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class FunctionDefinitionNode : public ASTNode
{
    ASTNode* body;  // Can be nullptr in case of declaration

    llvm::Function* define_function();

    void initialize_constructor(ClassType* class_type);

public:

    std::string name;

    FunctionDefinitionNode(ModuleCompiler* compiler, std::string name, ASTNode* body);

    void set_body(ASTNode* body) { assert(this->body == nullptr); this->body = body; }

    llvm::Function* eval() override;

    ~FunctionDefinitionNode() override;
};

}
