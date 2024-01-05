#pragma once

#include <AST/lvalue/VariableNode.hpp>

namespace dua
{

class ClassFieldNode : public VariableNode
{
    mutable Value instance_eval { nullptr, nullptr, nullptr };

    ASTNode* instance;

public:

    ClassFieldNode(ModuleCompiler* compiler, ASTNode* instance, std::string field_name)
            : VariableNode(compiler, std::move(field_name)), instance(instance) { }

    ClassFieldNode(ModuleCompiler* compiler, ASTNode* instance, std::string field_name, const std::vector<const Type*>& template_args)
            : VariableNode(compiler, std::move(field_name), template_args), instance(instance) { }

    Value eval() override;

    [[nodiscard]] const Type* get_type() override;

    [[nodiscard]] Value eval_instance() const;

    [[nodiscard]] std::string get_name() const override;

    [[nodiscard]] bool is_function() const override;
};

}
