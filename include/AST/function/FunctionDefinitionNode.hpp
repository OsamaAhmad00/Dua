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

    FunctionDefinitionNode(ModuleCompiler* compiler, std::string name, ASTNode* body,
                           const FunctionType* function_type, bool nomangle = false, size_t template_param_count = -1);

    void set_full_name();

    void set_body(ASTNode* body) { assert(this->body == nullptr); this->body = body; }

    FunctionDefinitionNode* clone() const;

    const FunctionType* get_function_type() { return function_type; }

    Value eval() override;
};

}
