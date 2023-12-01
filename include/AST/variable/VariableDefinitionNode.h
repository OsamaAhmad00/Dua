#pragma once

#include "AST/ASTNode.h"
#include "AST/values/ValueNode.h"

namespace dua
{

class VariableDefinitionNode : public ASTNode
{

protected:

    std::string name;
    ASTNode* initializer;
    // The type is already defined in the ASTNode class

public:

    VariableDefinitionNode(ModuleCompiler* compiler, std::string name, Type* type, ASTNode* initializer)
            : name(std::move(name)), initializer(initializer) { this->compiler = compiler; this->type = type; }
    llvm::Value* eval() override = 0;
    Type* compute_type() override { return type; }
    const std::string& get_name() const { return name; }
    ~VariableDefinitionNode() override { delete initializer; };
};

}
