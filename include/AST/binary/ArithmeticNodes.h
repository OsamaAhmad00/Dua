#pragma once

#include <AST/binary/BinaryExp.h>

BINARY_EXP_NODE(AdditionNode, CreateAdd, "temp_add")

BINARY_EXP_NODE(SubtractionNode, CreateSub, "temp_sub")

BINARY_EXP_NODE(MultiplicationNode, CreateMul, "temp_mul")

BINARY_EXP_NODE(DivisionNode, CreateSDiv, "temp_div")
