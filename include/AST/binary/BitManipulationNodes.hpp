#pragma once

#include <AST/binary/BinaryExp.hpp>

namespace dua
{

SAME_TYPE_BINARY_EXP_NODE(BitwiseAnd, CreateAnd, "temp_and")

SAME_TYPE_BINARY_EXP_NODE(Xor, CreateXor, "temp_xor")

SAME_TYPE_BINARY_EXP_NODE(BitwiseOr, CreateOr, "temp_or")

SAME_TYPE_BINARY_EXP_NODE(LeftShift, CreateShl, "temp_left_shift")

SAME_TYPE_BINARY_EXP_NODE(RightShift, CreateLShr, "temp_right_shift")

SAME_TYPE_BINARY_EXP_NODE(ArithmeticRightShift, CreateAShr, "temp_arithmetic_right_shift")

}
