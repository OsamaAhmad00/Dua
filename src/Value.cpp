#include <Value.hpp>
#include <TypingSystem.hpp>

namespace dua
{

Value::Value(TypingSystem *typing_system, llvm::Value *value, const Type *type, llvm::Value *memory_location)
    : typing_system(typing_system), loaded_value(value), type(type), memory_location(memory_location) {}

Value::Value(TypingSystem *typing_system, const Type *type, llvm::Value *memory_location)
    : Value(typing_system, nullptr, type, memory_location) {}

Value::Value() : Value(nullptr, nullptr, nullptr, nullptr) {}

Value Value::cast_as(const dua::Type *type, bool panic_on_failure) const {
    return typing_system->cast_value(*this, type, panic_on_failure);
}

Value Value::cast_as_bool(bool panic_on_failure) const {
    return typing_system->cast_as_bool(*this, panic_on_failure);
}

const Type *Value::get_winning_type(const Type *other, bool panic_on_failure) const {
    return typing_system->get_winning_type(this->type, other, panic_on_failure);
}

llvm::Constant* Value::get_constant() const {
    return as<llvm::Constant>();
}

bool Value::is_castable_as(const Type *type) const {
    return typing_system->is_castable(this->type, type);
}

bool Value::is_null() const {
    return !(loaded_value || memory_location);
}

llvm::Value* Value::get() const {
    if (loaded_value != nullptr) return loaded_value;
    if (memory_location == nullptr) return loaded_value;
    loaded_value = typing_system->builder().CreateLoad(type->llvm_type(), memory_location);
    return loaded_value;
}

void Value::set(llvm::Value *value) {
    loaded_value = value;
}

}