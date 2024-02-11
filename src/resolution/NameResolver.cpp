#include "resolution/NameResolver.hpp"
#include "ModuleCompiler.hpp"

namespace dua
{

NameResolver::NameResolver(ModuleCompiler *compiler) : compiler(compiler), FunctionNameResolver(compiler), symbol_table(compiler),
                                                       ClassResolver(compiler), TemplatedNameResolver(compiler) {}

void NameResolver::destruct_all_variables(const Scope<Value> &scope)
{
    size_t size = scope.map.size();
    if (size == 0) return;
    for (size_t i = size - 1; i != (size_t)-1; i--)
        call_destructor(scope.map[i].value);
}

void NameResolver::push_scope()
{
    symbol_table.push_scope();
}

Scope<Value> NameResolver::pop_scope()
{
    return symbol_table.pop_scope();
}

NameResolver::~NameResolver() {
    for (auto str : resolution_strings)
        delete str;
}

}