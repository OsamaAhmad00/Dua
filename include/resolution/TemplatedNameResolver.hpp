#pragma once

#include <types/FunctionType.hpp>
#include <resolution/CommonStructs.hpp>
#include <Value.hpp>
#include <unordered_map>
#include <vector>

namespace dua
{

class ModuleCompiler;
class ClassDefinitionNode;
class FunctionDefinitionNode;
class ClassType;

class TemplatedNameResolver
{
    friend class ParserAssistant;

    ModuleCompiler* compiler = nullptr;

    // A map of the templated functions, used to instantiate functions on demand
    std::unordered_map<std::string, std::vector<TemplatedFunctionNode>> templated_functions;

    // A map of the templated classes, used to instantiate classes on demand
    std::unordered_map<std::string, TemplatedClassNode> templated_classes;
    std::unordered_map<std::string, TemplatedClassMethodInfo> templated_class_method_info;
    std::unordered_map<std::string, TemplateBindings> templated_class_bindings;
    std::unordered_map<std::string, std::vector<TemplatedClassFieldConstructorArgs>> templated_class_field_constructor_args;

public:

    TemplatedNameResolver(ModuleCompiler* compiler) : compiler(compiler) {}

    std::string get_templated_function_key(std::string name, size_t args_count);
    std::string get_templated_function_full_name(std::string name, const std::vector<const Type*>& template_args);
    std::string get_templated_function_full_name(std::string name, const std::vector<const Type*>& template_args, const std::vector<const Type*>& param_types);
    void add_templated_function(FunctionDefinitionNode* node, std::vector<std::string> template_params, FunctionInfo info, const std::string& class_name = "", bool in_templated_class = false);
    Value get_templated_function(const std::string& name, std::vector<const Type*>& template_args);
    Value get_templated_function(const std::string& name, std::vector<const Type*>& template_args, const std::vector<const Type*>& arg_types, bool use_arg_types = true);
    long long get_winner_templated_function(const std::string& name, const std::vector<TemplatedFunctionNode>& functions, const std::vector<const Type*>& template_args, const std::vector<const Type*>& arg_types, bool panic_on_not_found = true);
    bool has_templated_function(const std::string& name);

    std::string get_templated_class_key(std::string name, size_t args_count);
    std::string get_templated_class_full_name(const std::string& name, const std::vector<const Type*>& template_args);
    void add_templated_class(ClassDefinitionNode* node, std::vector<std::string> template_params, const IdentifierType* parent);
    void add_templated_class_method_info(const std::string& cls, FunctionDefinitionNode* method, FunctionInfo info, std::vector<std::string> template_params);
    TemplatedClassMethodInfo get_templated_class_method_info(const std::string& cls, const std::string& method, const FunctionType* type, size_t template_param_count);
    const ClassType* get_templated_class(const std::string& name, const std::vector<const Type*>& template_args);
    void register_templated_class(const std::string& name, const std::vector<const Type*>& template_args);
    const ClassType* define_templated_class(const std::string& name, const std::vector<const Type*>& template_args);
    const ClassType* get_parent_class(const IdentifierType* parent);
    bool has_templated_class(const std::string& name);
};

}