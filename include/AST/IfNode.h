#pragma once

#include <AST/ASTNode.h>

class IfNode : public ASTNode
{
    // If a counter is not used, LLVM will assign numbers incrementally (for example then1, else2, condition3)
    //  which can be confusing, especially in nested expressions.
    static int _counter;

    std::vector<ASTNode*> conditions;
    // If branches.size() == conditions.size() + 1, branches.back() is the
    //  else branch. Otherwise, both sizes are equal.
    std::vector<ASTNode*> branches;

    // Used by when expressions
    std::string operation_name = "if";

    bool is_expression;

public:

    IfNode(ModuleCompiler* compiler, std::vector<ASTNode*> conditions,
           std::vector<ASTNode*> branches, bool is_expression=true, std::string operation_name="if")
        : conditions(std::move(conditions)),
          branches(std::move(branches)),
          is_expression(is_expression),
          operation_name(std::move(operation_name))
    {
        this->compiler = compiler;
    }

    llvm::Value* eval() override;

    bool has_else() { return branches.size() == conditions.size() + 1; }

    TypeBase* compute_type() override;

    ~IfNode() override;
};