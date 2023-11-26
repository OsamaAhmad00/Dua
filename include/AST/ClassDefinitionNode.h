#pragma once

#include <AST/ASTNode.h>

namespace dua
{

class ClassDefinitionNode : public ASTNode
{
    std::string name;
    std::vector<ASTNode*> members;

public:

    ClassDefinitionNode(ModuleCompiler* compiler, std::string name, std::vector<ASTNode*> members)
        : name(std::move(name)), members(std::move(members)) { this->compiler = compiler; }

    llvm::Value* eval() override;

    ~ClassDefinitionNode() override;
};

}
