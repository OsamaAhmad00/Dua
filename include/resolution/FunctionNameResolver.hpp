#pragma once

#include <map>
#include "types/FunctionType.hpp"
#include "types/ClassType.hpp"
#include <llvm/IR/IRBuilder.h>


namespace dua
{

class ModuleCompiler;
struct Value;

struct FunctionInfo
{
    const FunctionType* type;
    std::vector<std::string> param_names;
};

class FunctionNameResolver
{
    std::map<std::string, FunctionInfo> functions;

public:

    ModuleCompiler* compiler;

    explicit FunctionNameResolver(ModuleCompiler* compiler);

    [[nodiscard]] llvm::IRBuilder<>& builder() const;

    void register_function(std::string name, FunctionInfo info);
    FunctionInfo& get_function(const std::string& name);

    [[nodiscard]] bool has_function(const std::string& name) const;
    void cast_function_args(std::vector<llvm::Value*>& args, const FunctionType& type);
    llvm::CallInst* call_function(const std::string &name, std::vector<llvm::Value*> args = {});
    llvm::CallInst* call_function(llvm::Value* ptr, const FunctionType& type, std::vector<llvm::Value*> args = {});

    void call_constructor(const Variable& variable, std::vector<llvm::Value*> args);
    void call_destructor(const Variable& variable);

};

}