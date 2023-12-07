#include <types/Type.hpp>
#include <ModuleCompiler.hpp>

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

const Type* Type::get_winning_type(const Type *other, bool panic_on_failure) const {
    return compiler->typing_system.get_winning_type(this, other, panic_on_failure);
}

bool Type::is_castable(const Type *type) const {
    return compiler->typing_system.is_castable(this, type);
}

}