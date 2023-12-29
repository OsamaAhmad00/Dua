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
    friend class TemplatedNameResolver;

    std::string name;
    std::vector<ClassFieldDefinitionNode*> fields;
    std::vector<FunctionDefinitionNode*> methods;
    std::vector<TypeAliasNode*> aliases;
    bool is_packed;
    bool is_templated;

public:

    ClassDefinitionNode(ModuleCompiler* compiler, std::string name, std::vector<ClassFieldDefinitionNode*> fields = {},
                        std::vector<FunctionDefinitionNode*> methods = {}, std::vector<TypeAliasNode*> aliases = {},
                        bool is_packed = false, bool is_templated = false)
        : name(std::move(name)), fields(std::move(fields)), methods(std::move(methods)),
          aliases(std::move(aliases)), is_packed(is_packed), is_templated(is_templated)
    {
        this->compiler = compiler;
    }

    Value eval() override;
};

}
