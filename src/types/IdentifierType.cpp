#include <types/IdentifierType.hpp>
#include <ModuleCompiler.hpp>

namespace dua
{

const Type* IdentifierType::get_type() const {
    return compiler->get_typing_system().get_type(name);
}

Value IdentifierType::default_value() const {
    return get_type()->default_value();
}

llvm::Type *IdentifierType::llvm_type() const {
    return get_type()->llvm_type();
}

bool IdentifierType::operator==(const Type &other) {
    return *get_type() == other;
}

}