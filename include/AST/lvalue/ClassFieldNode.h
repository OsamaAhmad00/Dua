#pragma once

#include <AST/lvalue/LValueNode.h>

namespace dua
{

class ClassFieldNode : public LValueNode
{
    LValueNode* instance;
    std::string field_name;

public:

    ClassFieldNode(ModuleCompiler* compiler, LValueNode* instance, std::string field_name)
            : instance(instance), field_name(std::move(field_name)) { this->compiler = compiler; this->type = type; }

    llvm::Value* eval() override;

    TypeBase* compute_type() override;

    TypeBase* get_element_type() override;

    ~ClassFieldNode() override { delete instance; }
};

}
