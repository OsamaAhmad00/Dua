#include <types/IdentifierType.hpp>
#include <ModuleCompiler.hpp>

namespace dua
{

const Type* IdentifierType::get_concrete_type() const {
    return compiler->get_typing_system().get_type(name)->get_concrete_type();
}

Value IdentifierType::default_value() const {
    return get_concrete_type()->default_value();
}

llvm::Type *IdentifierType::llvm_type() const {
    return get_concrete_type()->llvm_type();
}

bool IdentifierType::operator==(const Type &other) const {
    return *get_concrete_type() == *other.get_concrete_type();
}

}