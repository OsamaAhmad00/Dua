#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

class ClassDefinitionNode : public ASTNode
{
    llvm::Value* result = nullptr;

    std::string name;
    std::vector<ASTNode*> members;
    bool is_packed;

public:

    ClassDefinitionNode(ModuleCompiler* compiler, std::string name, std::vector<ASTNode*> members = {}, bool is_packed = false)
        : name(std::move(name)), members(std::move(members)), is_packed(is_packed)
        { this->compiler = compiler; }

    llvm::Value* eval() override;
};

}
