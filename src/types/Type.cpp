#include <types/Type.hpp>
#include <ModuleCompiler.hpp>
#include "types/ReferenceType.hpp"

namespace dua
{

bool Type::operator==(const Type &other) {
     return llvm_type() == other.llvm_type();
}

bool Type::operator!=(const Type &other) {
     return !(*this == other);
}

llvm::Type *Type::operator->() const {
     return llvm_type();
}

Type::operator llvm::Type *() const {
     return llvm_type();
}

const Type* Type::get_winning_type(const Type *other, bool panic_on_failure, const std::string& message) const {
    return compiler->typing_system.get_winning_type(this, other, panic_on_failure, message);
}

bool Type::is_castable(const Type *type) const {
    return compiler->typing_system.is_castable(this, type);
}

const Type* Type::get_contained_type() const {
    auto result = this;
    if (auto ref = dynamic_cast<const ReferenceType*>(result); ref != nullptr)
        result = ref->get_element_type();
    if (auto i = dynamic_cast<const IdentifierType*>(result); i != nullptr)
        result = i->get_type();
    return (result == this) ? nullptr : result;
}

template <>
const ReferenceType* Type::as<ReferenceType>() const {
    return dynamic_cast<const ReferenceType*>(this);
}

}