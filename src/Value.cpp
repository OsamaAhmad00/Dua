#include <Value.hpp>
#include <TypingSystem.hpp>

namespace dua
{

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
    return typing_system == nullptr;
}

}