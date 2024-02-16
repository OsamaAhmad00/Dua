#pragma once

#include <AST/ASTNode.hpp>
#include "FunctionCallNode.hpp"

namespace dua
{

class MethodCallNode : public FunctionCallNode
{
    bool processed = false;
    bool is_callable_field = false;
    ASTNode* instance_node;

    void process();

    std::vector<const Type*> get_arg_types();

    const ClassType* get_instance_type();

    Value get_instance_ref();

    std::vector<Value> eval_args();

public:

    MethodCallNode(ModuleCompiler* compiler, ASTNode* instance_node,
                   ResolutionString* func_name, std::vector<ASTNode*> args, std::vector<const Type*> template_args)
            : FunctionCallNode(compiler, func_name, std::move(args), std::move(template_args)),
              instance_node(instance_node) {}

    MethodCallNode(ModuleCompiler* compiler, ASTNode* instance_node,
                   ResolutionString* func_name, std::vector<ASTNode*> args = {})
            : FunctionCallNode(compiler, func_name, std::move(args)),
                instance_node(instance_node) {}

    Value eval() override;

    const Type* get_type() override;
};

}
