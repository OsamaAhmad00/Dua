#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

class ClassFieldDefinitionNode;
class FunctionDefinitionNode;
class TypeAliasNode;

class ClassDefinitionNode : public ASTNode
{
    Value result;

    std::string name;
    std::vector<ClassFieldDefinitionNode*> fields;
    std::vector<FunctionDefinitionNode*> methods;
    std::vector<TypeAliasNode*> aliases;

public:

    ClassDefinitionNode(ModuleCompiler* compiler, std::string name, std::vector<ClassFieldDefinitionNode*> fields = {},
                        std::vector<FunctionDefinitionNode*> methods = {}, std::vector<TypeAliasNode*> aliases = {})
        : name(std::move(name)), fields(std::move(fields)), methods(std::move(methods)), aliases(std::move(aliases))
        { this->compiler = compiler; }

    Value eval() override;
};

}
