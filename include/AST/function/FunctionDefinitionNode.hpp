#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class FunctionDefinitionNode : public ASTNode
{
    ASTNode* body;  // Can be nullptr in case of declaration

    const FunctionType* function_type;

    Value define_function();

    void initialize_constructor(const ClassType* class_type);

public:

    bool is_templated;

    std::string name;

    FunctionDefinitionNode(ModuleCompiler* compiler, std::string name, ASTNode* body,
                           const FunctionType* function_type, bool is_templated = false);

    void set_body(ASTNode* body) { assert(this->body == nullptr); this->body = body; }

    const FunctionType* get_function_type() { return function_type; }

    Value eval() override;
};

}
