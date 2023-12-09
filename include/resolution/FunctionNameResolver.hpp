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


    void cast_function_args(std::vector<Value>& args, const FunctionType* type) const;

public:

    ModuleCompiler* compiler;

    explicit FunctionNameResolver(ModuleCompiler* compiler);

    [[nodiscard]] llvm::IRBuilder<>& builder() const;

    void register_function(std::string name, FunctionInfo info, bool no_mangle = false);
    // Used to resolve between applicable candidate functions
    [[nodiscard]] std::string get_winning_function(const std::string& name, const std::vector<const Type*>& arg_types) const;
    FunctionInfo& get_function(const std::string& name, const std::vector<const Type*>& param_types);
    FunctionInfo& get_function(const std::string& name, const std::vector<Value>& args);
    std::string get_function_with_exact_type(const std::string& name, const FunctionType* type) const;
    FunctionInfo& get_function_no_overloading(const std::string &name);
    std::string get_function(std::string name);

    [[nodiscard]] bool has_function(const std::string& name) const;
    llvm::CallInst* call_function(const std::string &name, std::vector<Value> args = {});
    llvm::CallInst* call_function(llvm::Value* ptr, const FunctionType* type, std::vector<Value> args = {});

    void call_constructor(const Value& value, std::vector<Value> args);
    void call_destructor(const Value& value);

    [[nodiscard]] static std::string get_full_function_name(std::string name, const std::vector<const Type*>& param_types);
};

}