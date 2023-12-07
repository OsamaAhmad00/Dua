#pragma once

#include <types/Type.hpp>
#include <llvm/IR/Type.h>

namespace dua
{

class TypingSystem;

struct Value
{
    TypingSystem* typing_system;
    llvm::Value* ptr;
    const Type* type;

    // Delegates to TypingSystem
    [[nodiscard]] llvm::Value* cast_as(const Type* type, bool panic_on_failure=true) const;
    [[nodiscard]] llvm::Value* cast_as_bool(bool panic_on_failure=true) const;
    [[nodiscard]] const Type* get_winning_type(const Type* other, bool panic_on_failure=true) const;
    [[nodiscard]] bool is_castable(const Type* type) const;
};

}

