#pragma once

#include <AST/ASTNode.h>
#include <AST/lvalue/LValueNode.h>
#include <AST/NoDeleteWrapperNode.h>

template <typename OpNode>
class OpAssignExpressionNode : public ASTNode
{
    LValueNode* lhs;
    ASTNode* rhs;

public:

    OpAssignExpressionNode(ModuleCompiler* compiler, LValueNode* lhs, ASTNode* rhs)
            : lhs(lhs), rhs(rhs) { this->compiler = compiler; };

    llvm::Value* eval() override
    {
        lhs->set_load_value(true);
        auto op = compiler->create_node<OpNode>(
            new NoDeleteWrapperNode<ASTNode>(lhs),
            new NoDeleteWrapperNode<ASTNode>(rhs)
        );
        auto value = op->eval();

        lhs->set_load_value(false);
        auto ptr = lhs->eval();
        builder().CreateStore(value, ptr);

        return value;
    }

    TypeBase* compute_type() override { delete type; return type = lhs->get_cached_type()->clone(); };

    ~OpAssignExpressionNode() override { delete lhs; delete rhs; };
};