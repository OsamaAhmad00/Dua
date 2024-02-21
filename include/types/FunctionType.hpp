#pragma once

#include <types/Type.hpp>
#include <llvm/IR/DerivedTypes.h>
#include <vector>

namespace dua
{

struct FunctionType : public Type
{
    llvm::FunctionType* llvm_type_cache = nullptr;
    const Type* return_type;
    std::vector<const Type*> param_types;
    bool is_var_arg;

    FunctionType(ModuleCompiler* compiler = nullptr, const Type* return_type = nullptr,
                 std::vector<const Type*> param_types = {}, bool is_var_arg = false)
            : return_type(return_type), param_types(std::move(param_types)), is_var_arg(is_var_arg)
    {
        this->compiler = compiler;
    }

    Value default_value() const override;

    const FunctionType* with_concrete_types() const;

    llvm::FunctionType* llvm_type() const override;

    std::string to_string() const override;

    std::string as_key() const override;

    bool operator==(const Type& other) const override;

    bool operator!=(const Type& other) const override { return !(*this == other); };
};

}
