#pragma once

#include <AST/binary/BinaryExp.hpp>

namespace dua
{

    class AdditionNode : public ASTNode {
        ASTNode *lhs;
        ASTNode *rhs;
    public:
        AdditionNode(ModuleCompiler *compiler, ASTNode *lhs, ASTNode *rhs) : lhs(lhs),
                                                                             rhs(rhs) { this->compiler = compiler; }
        static llvm::Value *perform(ModuleCompiler *compiler, const Value &lhs, const Value &rhs, const Type *type) {
            auto l = lhs.cast_as(type, false);
            auto r = rhs.cast_as(type, false);
            if (l == nullptr || r == nullptr)
                report_error(
                        "Can't perform the operation " "Addition" " between the types " + lhs.type->to_string() + " and " +
                        rhs.type->to_string());
            llvm::Value *ptr;
            if (dynamic_cast<const IntegerType *>(type))
                ptr = compiler->get_builder()->CreateAdd(l, r, "add_value");
            else {
                if (false)report_error("The operation " "Addition" " is applicable only on integer types");
                ptr = compiler->get_builder()->CreateFAdd(l, r, "add_value");
            }
            auto value = compiler->create_value(ptr, type);
            return value.cast_as(type);
        }
        llvm::Value *eval() override {
            auto lhs_value = lhs->get_eval_value();
            auto rhs_value = rhs->get_eval_value();
            auto infix_call = compiler->get_name_resolver().call_infix_operator(lhs_value, rhs_value, "Addition");
            if (infix_call != nullptr)return infix_call;
            return AdditionNode::perform(compiler, lhs_value, rhs_value, get_type());
        }
        const Type *get_type() override {
            if (type != nullptr)return type;
            auto ltype = lhs->get_type();
            auto rtype = rhs->get_type();
            auto infix_type = name_resolver().get_infix_operator_return_type(ltype, rtype, "Addition");
            if (infix_type != nullptr)return type = infix_type;
            return type = ltype->get_winning_type(rtype);
        }
    };
SAME_TYPE_BINARY_EXP_NODE(Subtraction, CreateSub, CreateFSub, "sub_value", false)

SAME_TYPE_BINARY_EXP_NODE(Multiplication, CreateMul, CreateFMul, "mul_value", false)

SAME_TYPE_BINARY_EXP_NODE(Division, CreateSDiv, CreateFDiv, "div_value", false)

SAME_TYPE_BINARY_EXP_NODE(Mod, CreateSRem, CreateFRem, "mod_value", false)

}