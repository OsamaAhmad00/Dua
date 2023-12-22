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

    std::string name;
    bool no_mangle;
    bool is_templated;

    FunctionDefinitionNode(ModuleCompiler* compiler, std::string name, ASTNode* body,
                           const FunctionType* function_type, bool no_mangle = false, bool is_templated = false);

    void set_full_name();

    void set_body(ASTNode* body) { assert(this->body == nullptr); this->body = body; }

    const FunctionType* get_function_type() { return function_type; }

    Value eval() override;
};

}
