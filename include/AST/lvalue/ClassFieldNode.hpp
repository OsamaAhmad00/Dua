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

    llvm::Value* eval() override;

    [[nodiscard]] const Type* get_type() override;

    [[nodiscard]] Value get_instance() const;

    [[nodiscard]] std::string get_full_name() const;

    [[nodiscard]] bool is_function() const override;
};

}
