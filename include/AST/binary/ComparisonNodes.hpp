#pragma once

#include <AST/binary/BinaryExp.hpp>

namespace dua
{

SAME_TYPE_BINARY_EXP_NODE(LTNode, CreateICmpSLT, "temp_lt")

SAME_TYPE_BINARY_EXP_NODE(GTNode, CreateICmpSGT, "temp_gt")

SAME_TYPE_BINARY_EXP_NODE(LTENode, CreateICmpSLE, "temp_lte")

SAME_TYPE_BINARY_EXP_NODE(GTENode, CreateICmpSGE, "temp_gte")

SAME_TYPE_BINARY_EXP_NODE(EQNode, CreateICmpEQ, "temp_eq")

SAME_TYPE_BINARY_EXP_NODE(NENode, CreateICmpNE, "temp_ne")

}
