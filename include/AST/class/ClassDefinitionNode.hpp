#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

class ClassFieldDefinitionNode;
class FunctionDefinitionNode;
class TypeAliasNode;

class ClassDefinitionNode : public ASTNode
{
    friend class ParserAssistant;

    Value result;

    std::string name;
    std::vector<ClassFieldDefinitionNode*> fields;
    std::vector<FunctionDefinitionNode*> methods;
    std::vector<TypeAliasNode*> aliases;
    bool is_packed;

public:

    ClassDefinitionNode(ModuleCompiler* compiler, std::string name, std::vector<ClassFieldDefinitionNode*> fields = {},
                        std::vector<FunctionDefinitionNode*> methods = {}, std::vector<TypeAliasNode*> aliases = {}, bool is_packed = false)
        : name(std::move(name)), fields(std::move(fields)), methods(std::move(methods)), aliases(std::move(aliases)), is_packed(is_packed)
        { this->compiler = compiler; }

    Value eval() override;
};

}
