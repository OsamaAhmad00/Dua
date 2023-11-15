#pragma once

#include <AST/ASTNode.h>
#include <types/TypeBase.h>

class LocalVariableDefinitionNode : public ASTNode
{
    std::string name;
    ASTNode* initializer;  // Can be nullptr in case of declaration
    // The type is already defined in the ASTNode class

public:
    LocalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, TypeBase* type, ASTNode* initializer)
            : name(std::move(name)), initializer(initializer) { this->compiler = compiler; this->type = type; }
    llvm::AllocaInst* eval() override;
    TypeBase* compute_type() override { return type; }
    ~LocalVariableDefinitionNode() override;
};