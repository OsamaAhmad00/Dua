#pragma once

#include <AST/binary/BinaryExp.hpp>

namespace dua
{

SAME_TYPE_BINARY_EXP_NODE(BitwiseAnd, CreateAnd, CreateAnd, "and_value", true)

SAME_TYPE_BINARY_EXP_NODE(Xor, CreateXor, CreateXor, "xor_value", true)

SAME_TYPE_BINARY_EXP_NODE(BitwiseOr, CreateOr, CreateOr, "or_value", true)

    class LeftShiftNode : public ASTNode {
        ASTNode *lhs;
        ASTNode *rhs;
    public:
        LeftShiftNode(ModuleCompiler *compiler, ASTNode *lhs, ASTNode *rhs) : lhs(lhs),
                                                                              rhs(rhs) { this->compiler = compiler; }
        static llvm::Value *perform(ModuleCompiler *compiler, const Value &lhs, const Value &rhs, const Type *type) {
            auto l = lhs.cast_as(type, false);
            auto r = rhs.cast_as(type, false);
            if (l == nullptr || r == nullptr)
                report_error(
                        "Can't perform the operation " "LeftShift" " between the types " + lhs.type->to_string() + " and " +
                        rhs.type->to_string());
            llvm::Value *ptr;
            if (dynamic_cast<const IntegerType *>(type))
                ptr = compiler->get_builder()->CreateShl(l, r, "left_shift_value");
            else {
                if (true)report_error("The operation " "LeftShift" " is applicable only on integer types");
                ptr = compiler->get_builder()->CreateShl(l, r, "left_shift_value");
            }
            auto value = compiler->create_value(ptr, type);
            return value.cast_as(type);
        }
        llvm::Value *eval() override {
            auto lhs_value = compiler->create_value(lhs->eval(), lhs->get_type());
            auto rhs_value = compiler->create_value(rhs->eval(), rhs->get_type());
            return LeftShiftNode::perform(compiler, lhs_value, rhs_value, get_type());
        }
        const Type *get_type() override {
            if (type != nullptr)return type;
            auto ltype = lhs->get_type();
            auto rtype = rhs->get_type();
            return type = ltype->get_winning_type(rtype);
        }
    };

SAME_TYPE_BINARY_EXP_NODE(RightShift, CreateLShr, CreateLShr, "right_shift_value", true)

SAME_TYPE_BINARY_EXP_NODE(ArithmeticRightShift, CreateAShr, CreateAShr, "arithmetic_right_shift_value", true)

}
