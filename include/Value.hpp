#pragma once

#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include "utils/ErrorReporting.hpp"

namespace dua
{

class TypingSystem;
class Type;

class Value
{
    mutable llvm::Value* loaded_value;  // Only loaded if needed

public:

    TypingSystem* typing_system = nullptr;
    const Type* type = nullptr;
    llvm::Value* memory_location = nullptr;

    Value(TypingSystem* typing_system, llvm::Value* value, const Type* type, llvm::Value* memory_location);
    Value(TypingSystem* typing_system, const Type* type, llvm::Value* memory_location);
    Value();

    void turn_to_memory_address();

    // Delegates to TypingSystem
    [[nodiscard]] Value cast_as(const Type* type, bool panic_on_failure=true) const;
    [[nodiscard]] Value cast_as_bool(bool panic_on_failure=true) const;
    [[nodiscard]] const Type* get_winning_type(const Type* other, bool panic_on_failure=true) const;
    [[nodiscard]] bool is_castable_as(const Type* type) const;
    [[nodiscard]] llvm::Constant* get_constant() const;
    [[nodiscard]] bool is_null() const;

    llvm::Value* get() const;
    void set(llvm::Value* value);

    template <typename T>
    [[nodiscard]] T* as() const {
        return llvm::dyn_cast<T>(get());
    }
};

}

