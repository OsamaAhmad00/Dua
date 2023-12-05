#include "types/PointerType.hpp"

namespace dua
{

llvm::Constant* PointerType::default_value() const {
    return llvm::Constant::getNullValue(llvm_type());
}

llvm::PointerType* PointerType::llvm_type() const {
    return element_type->llvm_type()->getPointerTo();
}

bool PointerType::operator==(const Type& other)
{
    if (auto casted = dynamic_cast<const PointerType*>(&other); casted != nullptr)
        return get_element_type() == casted->get_element_type();
    return false;
}

}