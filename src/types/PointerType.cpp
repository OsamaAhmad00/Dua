#include "types/PointerType.hpp"
#include <ModuleCompiler.hpp>
#include "types/ReferenceType.hpp"

namespace dua
{

PointerType::PointerType(ModuleCompiler *compiler, const Type *element_type)
{
    this->compiler = compiler;
    if (auto ref = element_type->as<ReferenceType>(); ref != nullptr)
        element_type = ref->get_element_type();
    this->element_type = element_type;
}

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
    return compiler->create_type<PointerType>(element_type->get_concrete_type()->get_contained_type());
}

}
