#pragma once

#include <AST/binary/BinaryExp.hpp>

namespace dua
{

SAME_TYPE_BINARY_EXP_NODE(Addition, CreateAdd, "temp_add")

SAME_TYPE_BINARY_EXP_NODE(Subtraction, CreateSub, "temp_sub")

SAME_TYPE_BINARY_EXP_NODE(Multiplication, CreateMul, "temp_mul")

SAME_TYPE_BINARY_EXP_NODE(Division, CreateSDiv, "temp_div")

SAME_TYPE_BINARY_EXP_NODE(Mod, CreateSRem, "temp_mod")

}