#pragma once

#include <AST/ASTNode.hpp>
#include "FunctionCallNode.hpp"

namespace dua
{

class MethodCallNode : public FunctionCallNode
{
    bool processed = false;
    bool is_callable_field = false;
    void process();

    std::string instance_name;

public:

    MethodCallNode(ModuleCompiler* compiler, std::string instance_name,
                   std::string func_name, std::vector<ASTNode*> args, std::vector<const Type*> template_args)
            : FunctionCallNode(compiler, std::move(func_name), std::move(args), std::move(template_args)),
              instance_name(std::move(instance_name)) {}

    MethodCallNode(ModuleCompiler* compiler, std::string instance_name,
                   std::string func_name, std::vector<ASTNode*> args = {})
            : FunctionCallNode(compiler, std::move(func_name), std::move(args)),
                instance_name(std::move(instance_name)) {}

    Value eval() override;

    const Type* get_type() override;
};

}
