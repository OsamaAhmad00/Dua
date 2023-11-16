#pragma once

#include <AST/binary/BinaryExp.h>

namespace dua
{

SAME_TYPE_BINARY_EXP_NODE(BitwiseAndNode, CreateAnd, "temp_and")

SAME_TYPE_BINARY_EXP_NODE(XorNode, CreateXor, "temp_xor")

SAME_TYPE_BINARY_EXP_NODE(BitwiseOrNode, CreateOr, "temp_or")

SAME_TYPE_BINARY_EXP_NODE(LeftShiftNode, CreateShl, "temp_left_shift")

SAME_TYPE_BINARY_EXP_NODE(RightShiftNode, CreateLShr, "temp_right_shift")

SAME_TYPE_BINARY_EXP_NODE(ArithmeticRightShiftNode, CreateAShr, "temp_arithmetic_right_shift")

}
