#pragma once

#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include "utils/ErrorReporting.hpp"

namespace dua
{

class TypingSystem;
class Type;

struct Value
{
    TypingSystem* typing_system = nullptr;
    llvm::Value* ptr = nullptr;
    const Type* type = nullptr;
    llvm::Value* memory_location = nullptr;

    // Delegates to TypingSystem
    [[nodiscard]] Value cast_as(const Type* type, bool panic_on_failure=true) const;
    [[nodiscard]] Value cast_as_bool(bool panic_on_failure=true) const;
    [[nodiscard]] const Type* get_winning_type(const Type* other, bool panic_on_failure=true) const;
    [[nodiscard]] bool is_castable_as(const Type* type) const;
    [[nodiscard]] llvm::Constant* get_constant() const;
    [[nodiscard]] bool is_null() const;

    template <typename T>
    [[nodiscard]] T* as() const {
        return llvm::dyn_cast<T>(ptr);
    }
};

}

