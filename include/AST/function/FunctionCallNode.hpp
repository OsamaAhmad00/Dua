#pragma once

#include <AST/function/FunctionCallBase.hpp>
#include <resolution/ResolutionString.hpp>

namespace dua
{

class FunctionCallNode : public FunctionCallBase
{
    friend class ParserAssistant;

protected:

    Value call_templated_function(std::vector<Value> args);

    Value call_reference(const Value& reference, std::vector<Value> args);

    Value get_method(std::vector<const Type*>& arg_types);

    Value get_templated_function(std::vector<const Type*>& args_types);

    std::string _current_name;

    ResolutionString* unresolved_name;
    std::vector<const Type*> template_args;
    bool is_templated;

public:

    FunctionCallNode(ModuleCompiler* compiler, ResolutionString* name, std::vector<ASTNode*> args = {});

    FunctionCallNode(ModuleCompiler* compiler, ResolutionString* name,
                     std::vector<ASTNode*> args, std::vector<const Type*> template_args);

    Value eval() override;

    const Type* get_type() override;
};

}
