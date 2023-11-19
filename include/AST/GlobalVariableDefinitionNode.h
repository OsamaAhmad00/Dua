#pragma once

#include <AST/ASTNode.h>
#include <AST/values/ValueNode.h>

namespace dua
{

class GlobalVariableDefinitionNode : public ASTNode
{
    std::string name;
    ASTNode* initializer;
    // The type is already defined in the ASTNode class

public:

    GlobalVariableDefinitionNode(ModuleCompiler* compiler, std::string name, TypeBase* type, ASTNode* initializer)
        : name(std::move(name)), initializer(initializer) { this->compiler = compiler; this->type = type; }
    llvm::GlobalVariable* eval() override;
    TypeBase* compute_type() override { return type; }
    ~GlobalVariableDefinitionNode() override;
};

}
