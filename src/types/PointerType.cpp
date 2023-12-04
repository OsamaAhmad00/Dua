#include "types/PointerType.hpp"

namespace dua
{

llvm::Constant* PointerType::default_value() {
    return llvm::Constant::getNullValue(llvm_type());
}

llvm::PointerType* PointerType::llvm_type() const {
    return element_type->llvm_type()->getPointerTo();
}

PointerType *PointerType::clone() {
    return new PointerType(compiler, element_type->clone());
}

}