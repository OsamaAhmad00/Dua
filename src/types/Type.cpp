#include <types/Type.hpp>
#include <ModuleCompiler.hpp>
#include "types/ReferenceType.hpp"
#include "types/IdentifierType.hpp"
#include "types/TypeOfType.hpp"

namespace dua
{

bool Type::operator==(const Type &other) const {
     return get_concrete_type() == other.get_concrete_type();
}

bool Type::operator!=(const Type &other) const {
     return !(*this == other);
}

const Type* Type::get_winning_type(const Type *other, bool panic_on_failure, const std::string& message) const {
    return compiler->typing_system.get_winning_type(this, other, panic_on_failure, message);
}

bool Type::is_castable(const Type *type) const {
    return compiler->typing_system.is_castable(this, type);
}

const Type* Type::get_contained_type() const
{
    // This is done in this sequence so that a reference of
    //  an identifier type would get stripped from the reference,
    //  then get the concrete type
    auto result = this;
    if (auto t = dynamic_cast<const TypeOfType*>(result); t != nullptr)
        result = t->get_concrete_type();
    if (auto ref = dynamic_cast<const ReferenceType*>(result); ref != nullptr)
        result = ref->get_element_type();
    if (auto i = dynamic_cast<const IdentifierType*>(result); i != nullptr)
        result = i->get_concrete_type();
    return result;
}

const Type *Type::get_concrete_type() const {
    return this;
}

bool Type::is_resolvable_now() const {
    auto concrete_type = get_concrete_type();
    return this == concrete_type || concrete_type->is_resolvable_now();
}

    template <>
const ReferenceType* Type::as<ReferenceType>() const {
    return dynamic_cast<const ReferenceType*>(this);
}

template <>
const IdentifierType* Type::as<IdentifierType>() const {
    return dynamic_cast<const IdentifierType*>(this);
}

}