#pragma once

#include <AST/ScopeTeleportingNode.hpp>

namespace dua
{

class FunctionCallNode : public ScopeTeleportingNode
{
    friend class ParserAssistant;

protected:

    Value call_templated_function(std::vector<Value> args);

    virtual std::vector<Value> eval_args();

    Value call_reference(const Value& reference, std::vector<Value> args);

    Value get_method(std::vector<const Type*>& arg_types);

    Value get_templated_function(std::vector<const Type*>& args_types);

    void set_teleporting_args(const std::vector<const Type*>& param_types, std::vector<Value>& evaluated_args);

    std::string name;
    std::vector<ASTNode*> args;
    std::vector<const Type*> template_args;
    bool is_templated;

public:

    FunctionCallNode(ModuleCompiler* compiler, std::string name,
                     std::vector<ASTNode*> args = {});
    FunctionCallNode(ModuleCompiler* compiler, std::string name,
                     std::vector<ASTNode*> args, std::vector<const Type*> template_args);

    Value eval() override;

    const Type* get_type() override;
};

}
