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

const Type *ArrayType::get_concrete_type() const {
    return compiler->create_type<ArrayType>(element_type->get_concrete_type(), size, _is_raw);
}

Value ArrayType::zero_value() const {
    std::vector<llvm::Constant*> values(size, element_type->zero_value().get_constant());
    auto result = llvm::ConstantArray::get(llvm_type(), values);
    return compiler->create_value(result, this);
}

bool ArrayType::operator==(const Type &other) const {
    auto as_arr = other.as<ArrayType>();
    return as_arr && element_type == as_arr->element_type && size == as_arr->size;
}

}
