#pragma once

#include <AST/binary/BinaryExp.hpp>

namespace dua
{

SAME_TYPE_BINARY_EXP_NODE(BitwiseAnd, CreateAnd, CreateAnd, "and_value", true)

SAME_TYPE_BINARY_EXP_NODE(Xor, CreateXor, CreateXor, "xor_value", true)

SAME_TYPE_BINARY_EXP_NODE(BitwiseOr, CreateOr, CreateOr, "or_value", true)

SAME_TYPE_BINARY_EXP_NODE(LeftShift, CreateShl, CreateShl, "left_shift_value", true)

SAME_TYPE_BINARY_EXP_NODE(RightShift, CreateLShr, CreateLShr, "right_shift_value", true)

SAME_TYPE_BINARY_EXP_NODE(ArithmeticRightShift, CreateAShr, CreateAShr, "arithmetic_right_shift_value", true)

}
