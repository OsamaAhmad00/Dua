#pragma once

#include "AST/ASTNode.hpp"

namespace dua
{

class LocalVariableDefinitionNode;
class FunctionDefinitionNode;

class ClassDefinitionNode : public ASTNode
{
    llvm::Value* result = nullptr;

    std::string name;
    std::vector<LocalVariableDefinitionNode*> fields;
    std::vector<FunctionDefinitionNode*> methods;

public:

    ClassDefinitionNode(ModuleCompiler* compiler, std::string name, std::vector<LocalVariableDefinitionNode*> fields = {},
                        std::vector<FunctionDefinitionNode*> methods = {})
        : name(std::move(name)), fields(std::move(fields)), methods(std::move(methods))
        { this->compiler = compiler; }

    llvm::Value* eval() override;
};

}
