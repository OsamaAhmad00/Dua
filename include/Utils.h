#pragma once

#include <llvm/IR/IRBuilder.h>

llvm::Value* cast_value(llvm::Value* value, llvm::Type* target_type, llvm::IRBuilder<>& builder);
