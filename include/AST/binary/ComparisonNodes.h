#pragma once

#include <AST/binary/BinaryExp.h>

BINARY_EXP_NODE(LTNode, CreateICmpSLT, "temp_lt")

BINARY_EXP_NODE(GTNode, CreateICmpSGT, "temp_gt")

BINARY_EXP_NODE(LTENode, CreateICmpSLE, "temp_lte")

BINARY_EXP_NODE(GTENode, CreateICmpSGE, "temp_gte")

BINARY_EXP_NODE(EQNode, CreateICmpEQ, "temp_eq")

BINARY_EXP_NODE(NENode, CreateICmpNE, "temp_ne")
