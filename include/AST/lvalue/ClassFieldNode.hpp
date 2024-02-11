#pragma once

#include <AST/lvalue/VariableNode.hpp>

namespace dua
{

class ClassFieldNode : public VariableNode
{
    mutable Value instance_eval { nullptr, nullptr, nullptr };

    ASTNode* instance;

    const ClassType* get_class(const Type* type) const;

    ResolutionString* get_resolution_name(ModuleCompiler* compiler, std::string name);

public:

    ClassFieldNode(ModuleCompiler* compiler, ASTNode* instance, std::string field_name)
            : VariableNode(compiler, get_resolution_name(compiler, std::move(field_name))), instance(instance) { }

    ClassFieldNode(ModuleCompiler* compiler, ASTNode* instance, std::string field_name, const std::vector<const Type*>& template_args)
            : VariableNode(compiler, get_resolution_name(compiler, std::move(field_name)), template_args), instance(instance) { }

    Value eval() override;

    [[nodiscard]] const Type* get_type() override;

    [[nodiscard]] Value eval_instance() const;

    [[nodiscard]] std::string get_name() const override;

    [[nodiscard]] bool is_function() const override;
};

}
