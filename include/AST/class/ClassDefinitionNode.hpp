#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

class ClassFieldDefinitionNode;
class FunctionDefinitionNode;

class ClassDefinitionNode : public ASTNode
{
    Value result;

    std::string name;
    std::vector<ClassFieldDefinitionNode*> fields;
    std::vector<FunctionDefinitionNode*> methods;

public:

    ClassDefinitionNode(ModuleCompiler* compiler, std::string name, std::vector<ClassFieldDefinitionNode*> fields = {},
                        std::vector<FunctionDefinitionNode*> methods = {})
        : name(std::move(name)), fields(std::move(fields)), methods(std::move(methods))
        { this->compiler = compiler; }

    Value eval() override;
};

}
