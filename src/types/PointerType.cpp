#include "types/PointerType.hpp"
#include <ModuleCompiler.hpp>

namespace dua
{

Value PointerType::default_value() const {
    return compiler->create_value(llvm::Constant::getNullValue(llvm_type()), this);
}

llvm::PointerType* PointerType::llvm_type() const {
    return element_type->llvm_type()->getPointerTo();
}

bool PointerType::operator==(const Type& other) const
{
    if (auto casted = other.as<PointerType>(); casted != nullptr)
        return get_element_type() == casted->get_element_type();
    return false;
}

const Type *PointerType::get_concrete_type() const {
    return compiler->create_type<PointerType>(element_type->get_concrete_type());
}

}