#include <llvm/IR/Constants.h>
#include "types/ArrayType.hpp"
#include <ModuleCompiler.hpp>

namespace dua
{

Value ArrayType::default_value() const {
    std::vector<llvm::Constant*> values(size, element_type->default_value().get_constant());
    auto result = llvm::ConstantArray::get(llvm_type(), values);
    return compiler->create_value(result, this);
}

llvm::ArrayType* ArrayType::llvm_type() const {
    return llvm::ArrayType::get(element_type->llvm_type(), size);
}

}
