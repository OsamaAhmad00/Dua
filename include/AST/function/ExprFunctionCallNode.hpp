#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class ExprFunctionCallNode : public ASTNode
{
    friend class ParserAssistant;

    const FunctionType* get_function_type(ASTNode* func);

protected:

    ASTNode* func;
    std::vector<ASTNode*> args;

public:

    ExprFunctionCallNode(ModuleCompiler* compiler, ASTNode* func, std::vector<ASTNode*> args = {})
        : func(func), args(std::move(args)) { this->compiler = compiler; }

    Value eval() override;

    const Type* get_type() override;
};

}
