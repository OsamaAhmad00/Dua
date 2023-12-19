#pragma once

#include <map>
#include "types/FunctionType.hpp"
#include "types/ClassType.hpp"
#include <llvm/IR/IRBuilder.h>


namespace dua
{

class ModuleCompiler;
class ASTNode;
class FunctionDefinitionNode;
struct Value;

struct FunctionInfo
{
    const FunctionType* type;
    std::vector<std::string> param_names;
};

struct TemplatedNode
{
    ASTNode* node;
    std::vector<std::string> template_params;
    FunctionInfo info;
};

class FunctionNameResolver
{
    std::map<std::string, FunctionInfo> functions;

    std::unordered_map<std::string, TemplatedNode> templated_functions;

    void cast_function_args(std::vector<Value>& args, const FunctionType* type) const;

public:

    ModuleCompiler* compiler;

    explicit FunctionNameResolver(ModuleCompiler* compiler);

    [[nodiscard]] llvm::IRBuilder<>& builder() const;

    void register_function(std::string name, const FunctionInfo& info, bool no_mangle = false);
    // Used to resolve between applicable candidate functions
    [[nodiscard]] std::string get_winning_function(const std::string& name, const std::vector<const Type*>& arg_types, bool panic_on_not_found = true) const;
    FunctionInfo& get_function(const std::string& name, const std::vector<const Type*>& param_types);
    FunctionInfo& get_function(const std::string& name, const std::vector<Value>& args);
    std::string get_function_with_exact_type(const std::string& name, const FunctionType* type) const;
    FunctionInfo& get_function_no_overloading(const std::string &name);
    std::string get_function(std::string name);

    [[nodiscard]] bool has_function(const std::string& name, bool try_as_method = true) const;
    Value call_function(const std::string &name, std::vector<Value> args = {});
    Value call_function(const Value& func, std::vector<Value> args = {});

    void call_constructor(const Value& value, std::vector<Value> args);
    void call_copy_constructor(const Value& value, const Value& arg);
    void call_destructor(const Value& value);

    Value call_infix_operator(const Value& lhs, const Value& rhs, const std::string& name);
    const Type* get_infix_operator_return_type(const Type* t1, const Type* t2, const std::string& name);

    [[nodiscard]] static std::string get_full_function_name(std::string name, const std::vector<const Type*>& param_types);

    std::string get_templated_function_full_name(std::string name, const std::vector<const Type*>& template_args);
    void add_templated_function(FunctionDefinitionNode* node, std::vector<std::string> template_params, FunctionInfo info);
    TemplatedNode& get_templated_function(std::string name, size_t template_arg_count);
};

}