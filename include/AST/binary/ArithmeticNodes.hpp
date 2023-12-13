#pragma once

#include <AST/binary/BinaryExp.hpp>

namespace dua
{

SAME_TYPE_BINARY_EXP_NODE(Addition, CreateAdd, CreateFAdd, "add_value", false)

SAME_TYPE_BINARY_EXP_NODE(Subtraction, CreateSub, CreateFSub, "sub_value", false)

SAME_TYPE_BINARY_EXP_NODE(Multiplication, CreateMul, CreateFMul, "mul_value", false)

SAME_TYPE_BINARY_EXP_NODE(Division, CreateSDiv, CreateFDiv, "div_value", false)

SAME_TYPE_BINARY_EXP_NODE(Mod, CreateSRem, CreateFRem, "mod_value", false)

}