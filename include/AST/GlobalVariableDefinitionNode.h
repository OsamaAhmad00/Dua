#pragma once

#include <AST/ASTNode.h>
#include <AST/terminals/ValueNode.h>

class GlobalVariableDefinitionNode : public ASTNode
{
    std::string name;
    TypeBase* type;
    ValueNode* initializer;

public:

    GlobalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, TypeBase* type, ValueNode* initializer)
        : name(std::move(name)), type(type), initializer(initializer) { this->compiler = compiler; }
    llvm::GlobalVariable* eval() override;
    ~GlobalVariableDefinitionNode() override;
};