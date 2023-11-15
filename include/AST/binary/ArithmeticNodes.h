#pragma once

#include <AST/binary/BinaryExp.h>

SAME_TYPE_BINARY_EXP_NODE(AdditionNode, CreateAdd, "temp_add")

SAME_TYPE_BINARY_EXP_NODE(SubtractionNode, CreateSub, "temp_sub")

SAME_TYPE_BINARY_EXP_NODE(MultiplicationNode, CreateMul, "temp_mul")

SAME_TYPE_BINARY_EXP_NODE(DivisionNode, CreateSDiv, "temp_div")

SAME_TYPE_BINARY_EXP_NODE(ModNode, CreateSRem, "temp_mod")
