#pragma once

#include <SymbolTable.hpp>
#include <resolution/FunctionNameResolver.hpp>
#include <resolution/TemplatedNameResolver.hpp>
#include <resolution/CommonStructs.hpp>
#include <types/FunctionType.hpp>
#include <types/ClassType.hpp>
#include <llvm/IR/IRBuilder.h>


namespace dua
{

class ASTNode;

class NameResolver : public FunctionNameResolver, public TemplatedNameResolver
{

public:

    ModuleCompiler* compiler;

    SymbolTable<Value> symbol_table;
    std::unordered_map<std::string, const ClassType*> classes;
    // Instead of having the fields be stored in the class type,
    //  and having them getting duplicated on each clone, let's
    //  keep the fields info in one place, and refer to it by name.
    std::unordered_map<std::string, std::vector<ClassField>> class_fields;
    std::unordered_map<std::string, std::vector<FieldConstructorArgs>> fields_args;

    explicit NameResolver(ModuleCompiler* compiler);


    const ClassType* get_class(const std::string& name);
    bool has_class(const std::string& name);
    void add_fields_constructor_args(std::string constructor_name, std::vector<FieldConstructorArgs> args);
    std::vector<FieldConstructorArgs>& get_fields_args(const std::string& constructor_name);

    void push_scope();
    Scope<Value> pop_scope();
    void destruct_all_variables(const Scope<Value>& scope);

};

}