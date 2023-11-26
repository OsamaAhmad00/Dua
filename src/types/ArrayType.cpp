#include <llvm/IR/Constants.h>
#include "types/ArrayType.h"

namespace dua
{

llvm::Constant* ArrayType::default_value() {
    std::vector<llvm::Constant*> values(size, element_type->default_value());
    return llvm::ConstantArray::get(llvm_type(), values);
}

llvm::ArrayType* ArrayType::llvm_type() const {
    return llvm::ArrayType::get(element_type->llvm_type(), size);
}

}
