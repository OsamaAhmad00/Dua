#include <types/PointerType.h>

llvm::Constant* PointerType::default_value() {
    return llvm::Constant::getNullValue(llvm_type());
}

llvm::PointerType* PointerType::llvm_type() {
    return element_type->llvm_type()->getPointerTo();
}

PointerType *PointerType::clone() {
    return new PointerType(builder, element_type->clone());
}
