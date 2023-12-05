#pragma once

#include <AST/binary/BinaryExp.hpp>

namespace dua
{

SAME_TYPE_BINARY_EXP_NODE(LT, CreateICmpSLT, "temp_lt")

SAME_TYPE_BINARY_EXP_NODE(GT, CreateICmpSGT, "temp_gt")

SAME_TYPE_BINARY_EXP_NODE(LTE, CreateICmpSLE, "temp_lte")

SAME_TYPE_BINARY_EXP_NODE(GTE, CreateICmpSGE, "temp_gte")

SAME_TYPE_BINARY_EXP_NODE(EQ, CreateICmpEQ, "temp_eq")

SAME_TYPE_BINARY_EXP_NODE(NE, CreateICmpNE, "temp_ne")

}
