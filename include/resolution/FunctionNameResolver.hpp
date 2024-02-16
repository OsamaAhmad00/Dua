#pragma once

#include <map>
#include "types/FunctionType.hpp"
#include "types/ClassType.hpp"
#include <llvm/IR/IRBuilder.h>
#include <resolution/CommonStructs.hpp>


namespace dua
{

class ModuleCompiler;
class ASTNode;
class FunctionDefinitionNode;
struct Value;

class FunctionNameResolver
{
    std::map<std::string, FunctionInfo> functions;

    void cast_function_args(std::vector<Value>& args, const FunctionType* type) const;
    void report_function_not_defined(const std::string& name);

public:

    ModuleCompiler* compiler;

    explicit FunctionNameResolver(ModuleCompiler* compiler);

    [[nodiscard]] llvm::IRBuilder<>& builder() const;

    void register_function(std::string name, FunctionInfo info, bool nomangle = false);
    // Used to resolve between applicable candidate functions
    [[nodiscard]] std::string get_winning_function(const std::string& name, const std::vector<const Type*>& arg_types, bool panic_on_not_found = true, bool panic_on_ambiguity = true) const;
    [[nodiscard]] std::string get_winning_method(const ClassType* owner, const std::string& name, const std::vector<const Type*>& arg_types, bool panic_on_not_found = true, bool panic_on_ambiguity = true) const;
    FunctionInfo& get_function(const std::string& name, const std::vector<const Type*>& param_types);
    FunctionInfo& get_function(const std::string& name, const std::vector<Value>& args);
    std::string get_function_name_with_exact_type(const std::string& name, const FunctionType* type) const;
    FunctionInfo& get_function_no_overloading(const std::string &name);
    std::vector<NamedFunctionValue> get_class_methods(std::string name, bool for_a_vtable = false);
    bool is_function_templated(const std::string& name);
    bool is_function_a_constructor(const std::string& name);

    [[nodiscard]] bool has_function(const std::string& name, bool try_as_method = true) const;
    Value call_function(const std::string &name, std::vector<Value> args = {}, Value* out_result = nullptr);
    Value call_function(const Value& func, std::vector<Value> args = {});

    void call_constructor(const Value& value, std::vector<Value> args);
    void copy_construct(const Value& value, const Value& arg);
    void call_destructor(const Value& value);

    Value call_operator(const std::string& position_name, const Value& lhs, const Value& rhs, const std::string& name);
    Value call_infix_operator(const Value& lhs, const Value& rhs, const std::string& name);
    Value call_postfix_operator(const Value& lhs, const Value& rhs, const std::string& name);
    const Type* get_operator_return_type(const std::string& position_name, const Type* t1, const Type* t2, const std::string& name);
    const Type* get_infix_operator_return_type(const Type* t1, const Type* t2, const std::string& name);
    const Type* get_postfix_operator_return_type(const Type* t1, const Type* t2, const std::string& name);

    [[nodiscard]] static std::string get_function_full_name(std::string name, const std::vector<const Type*>& param_types);
    [[nodiscard]] std::string get_function_full_name(std::string name, bool try_as_method = true, bool panic_on_error = false);

};

}