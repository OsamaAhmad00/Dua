#pragma once

#include <AST/function/FunctionCallBase.hpp>

namespace dua
{

class ExprFunctionCallNode : public FunctionCallBase
{
    friend class ParserAssistant;

    const FunctionType* get_function_type(ASTNode* func);

protected:

    ASTNode* func;
public:

    ExprFunctionCallNode(ModuleCompiler* compiler, ASTNode* func, std::vector<ASTNode*> args = {})
        : func(func), FunctionCallBase(compiler, std::move(args)) { }

    Value eval() override;

    const Type* get_type() override;
};

}
