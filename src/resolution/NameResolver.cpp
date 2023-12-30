#include "resolution/NameResolver.hpp"
#include "ModuleCompiler.hpp"

namespace dua
{

NameResolver::NameResolver(ModuleCompiler *compiler) : compiler(compiler), FunctionNameResolver(compiler),
                                                       ClassResolver(compiler), TemplatedNameResolver(compiler) {}

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

}