#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class FunctionDefinitionNode : public ASTNode
{
    friend class TemplatedNameResolver;

    ASTNode* body;  // Can be nullptr in case of declaration

    const FunctionType* function_type;

    Value define_function();

    void construct_fields(const ClassType* class_type);

    void destruct_fields(const ClassType* class_type);

public:

    static constexpr int NOT_TEMPLATED = -1;
    static constexpr int TEMPLATED_BUT_EVALUATE = -2;

    std::string name;
    bool nomangle;
    int template_param_count;
    // The function name doesn't get prefixed with
    // the name of the owner class if it's an operator
    bool is_operator;
    bool is_static;

    FunctionDefinitionNode(ModuleCompiler* compiler, std::string name, ASTNode* body,
                           const FunctionType* function_type, bool nomangle = false,
                           size_t template_param_count = NOT_TEMPLATED, bool is_operator = false, bool is_static = false);

    void set_full_name();

    void set_body(ASTNode* body) { assert(this->body == nullptr); this->body = body; }

    FunctionDefinitionNode* clone() const;

    const FunctionType* get_function_type() { return function_type; }

    Value eval() override;
};

}
