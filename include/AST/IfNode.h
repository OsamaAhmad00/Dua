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

public:

    IfNode(ModuleCompiler* compiler, std::vector<ASTNode*> conditions, std::vector<ASTNode*> branches)
        : conditions(std::move(conditions)), branches(std::move(branches)) { this->compiler = compiler; }
    llvm::PHINode* eval() override;
    bool has_else() { return branches.size() == conditions.size() + 1; }
    ~IfNode() override;
};