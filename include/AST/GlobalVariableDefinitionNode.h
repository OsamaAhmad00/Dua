#pragma once

#include <AST/ASTNode.h>
#include <AST/terminals/ValueNode.h>

class GlobalVariableDefinitionNode : public ASTNode
{
    std::string name;
    ValueNode* initializer;
    // The type is already defined in the ASTNode class

public:

    GlobalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, TypeBase* type, ValueNode* initializer)
        : name(std::move(name)), initializer(initializer) { this->compiler = compiler; this->type = type; }
    llvm::GlobalVariable* eval() override;
    TypeBase* compute_type() override { return type; }
    ~GlobalVariableDefinitionNode() override;
};