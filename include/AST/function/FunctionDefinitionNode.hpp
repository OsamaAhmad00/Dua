#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class FunctionDefinitionNode : public ASTNode
{
    friend class ModuleCompiler;

    ASTNode* body;  // Can be nullptr in case of declaration

    const FunctionType* function_type;

    Value define_function();

    void initialize_constructor(const ClassType* class_type);

public:

    std::string name;
    bool nomangle;
    size_t template_param_count;
    // The function name doesn't get prefixed with
    // the name of the owner class if it's an operator
    bool is_operator;

    FunctionDefinitionNode(ModuleCompiler* compiler, std::string name, ASTNode* body,
                           const FunctionType* function_type, bool nomangle = false, size_t template_param_count = -1, bool is_operator = false);

    void set_full_name();

    void set_body(ASTNode* body) { assert(this->body == nullptr); this->body = body; }

    FunctionDefinitionNode* clone() const;

    const FunctionType* get_function_type() { return function_type; }

    Value eval() override;
};

}
