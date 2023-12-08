#pragma once

#include <AST/binary/BinaryExp.hpp>

namespace dua
{

SAME_TYPE_BINARY_EXP_NODE(LT, CreateICmpSLT, CreateFCmpULT, "lt_value", false)

SAME_TYPE_BINARY_EXP_NODE(GT, CreateICmpSGT, CreateFCmpUGT, "gt_value", false)

SAME_TYPE_BINARY_EXP_NODE(LTE, CreateICmpSLE, CreateFCmpULE, "lte_value", false)

SAME_TYPE_BINARY_EXP_NODE(GTE, CreateICmpSGE, CreateFCmpUGE, "gte_value", false)

SAME_TYPE_BINARY_EXP_NODE(EQ, CreateICmpEQ, CreateFCmpUEQ, "eq_value", false)

SAME_TYPE_BINARY_EXP_NODE(NE, CreateICmpNE, CreateFCmpUNE, "temp_ne", false)

}
