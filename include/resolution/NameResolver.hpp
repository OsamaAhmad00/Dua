#pragma once

#include <SymbolTable.hpp>
#include <resolution/FunctionNameResolver.hpp>
#include <resolution/ClassResolver.hpp>
#include <resolution/TemplatedNameResolver.hpp>
#include <resolution/CommonStructs.hpp>


namespace dua
{

class ASTNode;

class NameResolver : public FunctionNameResolver, public ClassResolver, public TemplatedNameResolver
{

public:

    ModuleCompiler* compiler;

    SymbolTable<Value> symbol_table;

    explicit NameResolver(ModuleCompiler* compiler);

    void push_scope();
    Scope<Value> pop_scope();
    void destruct_all_variables(const Scope<Value>& scope);

};

}