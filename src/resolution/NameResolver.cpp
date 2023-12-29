#include "resolution/NameResolver.hpp"
#include "ModuleCompiler.hpp"

namespace dua
{

NameResolver::NameResolver(ModuleCompiler *compiler) : compiler(compiler), FunctionNameResolver(compiler),
                                                       TemplatedNameResolver(compiler) {}

void NameResolver::add_fields_constructor_args(std::string constructor_name, std::vector<FieldConstructorArgs> args)
{
    fields_args[std::move(constructor_name)] = std::move(args);
}

std::vector<FieldConstructorArgs>& NameResolver::get_fields_args(const std::string &constructor_name) {
    auto it = fields_args.find(constructor_name);
    if (it == fields_args.end()) {
        // This constructor has no arguments for fields, return an empty one
        return fields_args[""];
    }
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
    auto it = classes.find(name);
    if (it == classes.end())
        report_internal_error("The class " + name + " is not defined");
    return it->second;
}

bool NameResolver::has_class(const std::string &name) {
    return classes.find(name) != classes.end();
}

}