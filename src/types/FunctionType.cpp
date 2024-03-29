#include <types/FunctionType.hpp>
#include <ModuleCompiler.hpp>
#include "types/ReferenceType.hpp"


namespace dua
{

Value FunctionType::default_value() const {
    return compiler->create_value(llvm::Constant::getNullValue(llvm_type()), this);
}

llvm::FunctionType* FunctionType::llvm_type() const
{
    if (llvm_type_cache != nullptr) return llvm_type_cache;
    llvm::Type* ret = return_type->llvm_type();
    std::vector<llvm::Type*> params(param_types.size());
    for (size_t i = 0; i < param_types.size(); i++) {
        params[i] = param_types[i]->llvm_type();
    }
    return llvm::FunctionType::get(ret, std::move(params), is_var_arg);
}

std::string FunctionType::to_string() const
{
    std::string result = return_type->to_string() + "(";

    if (!param_types.empty()) {
        for (size_t i = 0; i < param_types.size() - 1; i++)
            result += param_types[i]->to_string() + ", ";
        result += param_types.back()->to_string();
    }

    if (is_var_arg)
        result += ", ...";

    result += ")";

    return result;
}

std::string FunctionType::as_key() const
{
    std::string result = return_type->as_key() + "__";

    if (!param_types.empty()) {
        for (size_t i = 0; i < param_types.size() - 1; i++)
            result += param_types[i]->as_key() + "_";
        result += param_types.back()->as_key();
    }

    if (is_var_arg)
        result += "_...";

    result += "__";

    return result;
}

bool FunctionType::operator==(const Type &other) const
{
    auto other_func = other.as<FunctionType>();
    if (other_func == nullptr) return false;
    if (*return_type != *other_func->return_type) return false;
    if (param_types.size() != other_func->param_types.size()) return false;
    // The names can be different.
    for (size_t i = 0; i < param_types.size(); i++)
        if (*param_types[i] != *other_func->param_types[i])
            return false;
    return is_var_arg == other_func->is_var_arg;
}

const FunctionType *FunctionType::with_concrete_types() const
{
    auto new_return_type = return_type->get_concrete_type();

    std::vector<const Type*> new_param_types(param_types.size());
    for (size_t i = 0; i < param_types.size(); i++)
        new_param_types[i] = param_types[i]->get_concrete_type();

    if (return_type != new_return_type || param_types != new_param_types)
        return compiler->create_type<FunctionType>(new_return_type, new_param_types, is_var_arg);

    return this;
}

const Type* FunctionType::get_concrete_type() const {
    return with_concrete_types();
}

}
