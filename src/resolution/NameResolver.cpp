#include "resolution/NameResolver.hpp"
#include "ModuleCompiler.hpp"

namespace dua
{

struct FieldInfo
{
    size_t index;
    ClassField& info;
};

NameResolver::NameResolver(ModuleCompiler *compiler) : compiler(compiler), function_resolver(compiler) {}

llvm::IRBuilder<>& NameResolver::builder() const
{
    return compiler->builder;
}

void NameResolver::add_fields_constructor_args(std::string class_name, std::vector<FieldConstructorArgs> args)
{
    fields_args[std::move(class_name)] = std::move(args);
}

std::vector<FieldConstructorArgs> &NameResolver::get_fields_args(const std::string &class_name) {
    auto it = fields_args.find(class_name);
    if (it == fields_args.end())
        report_internal_error("Class " + class_name + " is not defined yet");
    return it->second;
}

void NameResolver::destruct_all_variables(const Scope<Value> &scope)
{
    // TODO enforce an order on the destruction
    for (auto& variable : scope.map)
        call_destructor(variable.second);
}

void NameResolver::push_scope()
{
    symbol_table.push_scope();
}

Scope<Value> NameResolver::pop_scope()
{
    return symbol_table.pop_scope();
}

const ClassType *NameResolver::get_class(const std::string &name) {
     return classes[name];
}

void NameResolver::register_function(std::string name, FunctionInfo info) {
    return function_resolver.register_function(std::move(name), std::move(info));
}

FunctionInfo &NameResolver::get_function(const std::string &name) {
    return function_resolver.get_function(name);
}

bool NameResolver::has_function(const std::string &name) const {
    return function_resolver.has_function(name);
}

llvm::CallInst *NameResolver::call_function(const std::string &name, std::vector<Value> args) {
    return function_resolver.call_function(name, std::move(args));
}

llvm::CallInst* NameResolver::call_function(llvm::Value *ptr, const FunctionType* type, std::vector<Value> args) {
    return function_resolver.call_function(ptr, type, std::move(args));
}

void NameResolver::call_constructor(const Value &value, std::vector<Value> args) {
    return function_resolver.call_constructor(value, std::move(args));
}

void NameResolver::call_destructor(const Value &value) {
    return function_resolver.call_destructor(value);
}

}