#pragma once

#include <AST/lvalue/VariableNode.h>

namespace dua
{

class ClassFieldNode : public VariableNode
{
    llvm::Value* instance_eval = nullptr;

    LValueNode* instance;

public:

    ClassFieldNode(ModuleCompiler* compiler, LValueNode* instance, std::string field_name)
            : VariableNode(compiler, std::move(field_name)), instance(instance) { }

    llvm::Value* eval() override;

    Type* compute_type() override;

    llvm::Value* get_instance();

    std::string get_full_name() const;

    bool is_function() const override;

    ~ClassFieldNode() override { delete instance; }
};

}
