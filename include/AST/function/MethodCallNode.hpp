#pragma once

#include <AST/ASTNode.hpp>
#include "FunctionCallNode.hpp"
#include "AST/lvalue/LValueNode.hpp"

namespace dua
{

class MethodCallNode : public FunctionCallNode
{
    bool processed = false;
    bool is_callable_field = false;
    void process();

    ASTNode* instance_node;

    std::vector<const Type*> get_arg_types();
    const ClassType* get_instance_type();

public:

    MethodCallNode(ModuleCompiler* compiler, ASTNode* instance_node,
                   std::string func_name, std::vector<ASTNode*> args, std::vector<const Type*> template_args)
            : FunctionCallNode(compiler, std::move(func_name), std::move(args), std::move(template_args)),
              instance_node(instance_node) {}

    MethodCallNode(ModuleCompiler* compiler, ASTNode* instance_node,
                   std::string func_name, std::vector<ASTNode*> args = {})
            : FunctionCallNode(compiler, std::move(func_name), std::move(args)),
                instance_node(instance_node) {}

    Value eval() override;

    const Type* get_type() override;
};

}
