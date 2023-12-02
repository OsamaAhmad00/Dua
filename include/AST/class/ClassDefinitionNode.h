#pragma once

#include "AST/ASTNode.h"

namespace dua
{

class ClassDefinitionNode : public ASTNode
{
    llvm::Value* result = nullptr;

    std::string name;
    std::vector<ASTNode*> members;
    std::vector<FieldConstructorArgs> fields_args;

public:

    ClassDefinitionNode(ModuleCompiler* compiler, std::string name,
        std::vector<ASTNode*> members = {}, std::vector<FieldConstructorArgs> fields_args = {})
        : name(std::move(name)), members(std::move(members)), fields_args(std::move(fields_args))
        { this->compiler = compiler; }

    llvm::Value* eval() override;

    ~ClassDefinitionNode() override;
};

}
