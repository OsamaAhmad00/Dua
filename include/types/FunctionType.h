#pragma once

#include <types/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <vector>

namespace dua
{

struct FunctionType : public Type
{
    Type* return_type;
    std::vector<Type*> param_types;
    bool is_var_arg;

    FunctionType(ModuleCompiler* compiler = nullptr, Type* return_type = nullptr,
                 std::vector<Type*> param_types = {}, bool is_var_arg = false)
            : return_type(return_type), param_types(std::move(param_types)), is_var_arg(is_var_arg)
    {
        this->compiler = compiler;
    }

    FunctionType(FunctionType&& other) { *this = std::move(other); }

    FunctionType& operator=(FunctionType&& other);

    llvm::Constant* default_value() override;

    llvm::FunctionType* llvm_type() const override;

    FunctionType* clone() override;

    std::string to_string() const override;

    bool operator==(const FunctionType& other) const;

    bool operator!=(const FunctionType& other) const { return !(*this == other); };

    ~FunctionType() override;
};

}
