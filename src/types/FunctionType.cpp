#include <types/FunctionType.hpp>


namespace dua
{

inline bool equal_types(Type* t1, Type* t2) {
    return (bool)t1 == (bool)t2 && (!t1 || *t1 == *t2);
}

llvm::Constant* FunctionType::default_value() {
    return llvm::Constant::getNullValue(llvm_type());
}

llvm::FunctionType* FunctionType::llvm_type() const
{
    llvm::Type* ret = return_type->llvm_type();
    std::vector<llvm::Type*> params(param_types.size());
    for (size_t i = 0; i < param_types.size(); i++)
        params[i] = param_types[i]->llvm_type();
    return llvm::FunctionType::get(ret, std::move(params), is_var_arg);
}

FunctionType *FunctionType::clone()
{
    Type* ret = return_type->clone();
    std::vector<Type*> params(param_types.size());
    for (size_t i = 0; i < param_types.size(); i++)
        params[i] = param_types[i]->clone();
    return new FunctionType(compiler, ret, std::move(params), is_var_arg);
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

FunctionType &FunctionType::operator=(FunctionType &&other)
{
    param_types = std::move(other.param_types);
    return_type = other.return_type;
    is_var_arg = other.is_var_arg;
    other.param_types.clear();
    other.return_type = nullptr;
    return *this;
}

FunctionType::~FunctionType()
{
    delete return_type;
    for (auto& param : param_types)
        delete param;
}

}
