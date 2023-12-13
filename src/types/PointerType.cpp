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

bool PointerType::operator==(const Type& other)
{
    if (auto casted = dynamic_cast<const PointerType*>(&other); casted != nullptr)
        return get_element_type() == casted->get_element_type();
    return false;
}

}