#pragma once

#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include "utils/ErrorReporting.hpp"

#define VALUE_DEFAULT_ID -1

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
    bool is_teleporting = false;

    // Used when the value needs to be tracked.
    //  For example, when tracking an expression
    //  that creates an object, such as a function
    //  call that returns an object. In this case,
    //  we need to track the object value to determine
    //  whether the object is bound or not, and
    //  call the destructor accordingly.
    int64_t id = VALUE_DEFAULT_ID;

    Value(TypingSystem* typing_system, llvm::Value* value, const Type* type, llvm::Value* memory_location);
    Value(TypingSystem* typing_system, const Type* type, llvm::Value* memory_location);
    Value();

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

