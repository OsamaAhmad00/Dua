#pragma once

#include <AST/ASTNode.h>
#include <types/TypeBase.h>

class LocalVariableDefinitionNode : public ASTNode
{
    std::string name;
    TypeBase* type;
    ASTNode* initializer;  // Can be nullptr in case of declaration

public:
    LocalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, TypeBase* type, ASTNode* initializer)
            : name(std::move(name)), type(type), initializer(initializer) { this->compiler = compiler; }
    llvm::AllocaInst* eval() override;
    ~LocalVariableDefinitionNode() override;
};