#pragma once

#include <SymbolTable.hpp>
#include <resolution/FunctionNameResolver.hpp>
#include <resolution/ClassResolver.hpp>
#include <resolution/TemplatedNameResolver.hpp>
#include <resolution/CommonStructs.hpp>
#include <resolution/ResolutionString.hpp>


namespace dua
{

class ASTNode;

class NameResolver : public FunctionNameResolver, public ClassResolver, public TemplatedNameResolver
{

public:

    ModuleCompiler* compiler;

    SymbolTable<Value> symbol_table;
    std::vector<ResolutionString*> resolution_strings;

    explicit NameResolver(ModuleCompiler* compiler);

    void push_scope();
    Scope<Value> pop_scope();
    void destruct_all_variables(const Scope<Value>& scope);

    template<typename T, typename ...Args>
    ResolutionString* create_resolution_string(Args ...args) {
        return new T(compiler, args...);
    }

    ~NameResolver();
};

}