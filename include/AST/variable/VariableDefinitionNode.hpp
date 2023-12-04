#pragma once

#include "AST/ASTNode.hpp"
#include "AST/values/ValueNode.hpp"

namespace dua
{

class VariableDefinitionNode : public ASTNode
{

protected:

    std::string name;
    ASTNode* initializer;
    // The type is already defined in the ASTNode class
    std::vector<ASTNode*> args;

public:

    VariableDefinitionNode(ModuleCompiler* compiler, std::string name, Type* type,
                           ASTNode* initializer, std::vector<ASTNode*> args)
            : name(std::move(name)), initializer(initializer), args(std::move(args))
    { this->compiler = compiler; this->type = type; }
    llvm::Value* eval() override = 0;
    Type* compute_type() override { return type; }
    const std::string& get_name() const { return name; }
    ~VariableDefinitionNode() override { delete initializer; for (auto& arg : args) delete arg; };
};

}
