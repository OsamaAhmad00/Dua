#include <types/FunctionType.hpp>
#include <ModuleCompiler.hpp>
#include "types/ReferenceType.hpp"


namespace dua
{

inline bool equal_types(const Type* t1, const Type* t2) {
    return (bool)t1 == (bool)t2 && (!t1 || *t1 == *t2);
}

Value FunctionType::default_value() const {
    return compiler->create_value(llvm::Constant::getNullValue(llvm_type()), this);
}

llvm::Type* get_llvm_representation(const Type* type)
{
    // Reference types are represented as pointers, and the address of the
    //  referenced variable is passed. This won't create collision between
    //  a function that accepts a pointer, and a function that accepts a
    //  reference, because the dua::Type of the two parameters is still different.
    if (auto ref = type->as<ReferenceType>(); ref != nullptr)
        return ref->get_element_type()->llvm_type()->getPointerTo();
    return type->llvm_type();
}

llvm::FunctionType* FunctionType::llvm_type() const
{
    if (llvm_type_cache != nullptr) return llvm_type_cache;
    llvm::Type* ret = get_llvm_representation(return_type);
    std::vector<llvm::Type*> params(param_types.size());
    for (size_t i = 0; i < param_types.size(); i++) {
        params[i] = get_llvm_representation(param_types[i]);
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

bool FunctionType::operator==(const FunctionType &other) const
{
    if (!equal_types(return_type, other.return_type))
        return false;
    if (param_types.size() != other.param_types.size())
        return false;
    // The names can be different.
    for (size_t i = 0; i < param_types.size(); i++)
        if (!equal_types(param_types[i], other.param_types[i]))
            return false;
    return is_var_arg == other.is_var_arg;
}

}
