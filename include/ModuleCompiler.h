#pragma once

#include <string>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <SymbolTable.h>
#include <types/Type.h>
#include <types/ClassType.h>
#include "types/FunctionType.h"

namespace dua
{

class ASTNode;

struct FieldConstructorArgs
{
    std::string name;
    std::vector<ASTNode*> args;
};

struct FunctionInfo
{
    FunctionType type;
    std::vector<std::string> param_names;
};

struct Variable
{
    llvm::Value* ptr;
    Type* type;
};

class ModuleCompiler
{
public:

    friend class ASTNode;
    friend class ParserAssistant;

    ModuleCompiler(const std::string& module_name, const std::string& code);

    // Returns the result type of an operation involving the two types.
    Type* get_winning_type(Type* lhs, Type* rhs);

    llvm::Value* cast_value(llvm::Value* value, llvm::Type* target_type, bool panic_on_failure=true);
    llvm::Value* cast_as_bool(llvm::Value* value, bool panic_on_failure=true);

    const std::string& get_result() { return result; }

    ~ModuleCompiler() {
        for (auto& cls : classes)
            delete cls.second;
    }

    template <typename T, typename ...Args>
    T* create_node(Args ...args)
    {
        return new T(this, args...);
    }

    template <typename T, typename ...Args>
    T* create_type(Args ...args)
    {
        return new T(this, args...);
    }

    void register_function(std::string name, FunctionInfo&& signature);
    FunctionInfo& get_function(const std::string& name);

    llvm::IRBuilder<>* get_builder() { return &builder; }
    llvm::LLVMContext* get_context() { return &context; }
    void push_deferred_node(ASTNode* node) { deferred_nodes.push_back(node); }
    auto get_class(const std::string& name) { return classes[name]; }
    auto& get_class_fields() { return class_fields; }
    void add_fields_constructor_args(std::string class_name, std::vector<FieldConstructorArgs> args);
    std::vector<FieldConstructorArgs>& get_fields_args(const std::string& class_name);

    bool has_function(const std::string& name) const;
    void cast_function_args(std::vector<llvm::Value*>& args, const FunctionType& type);
    llvm::CallInst* call_function(const std::string &name, std::vector<llvm::Value*> args = {});
    llvm::CallInst* call_function(llvm::Value* ptr, const FunctionType& type, std::vector<llvm::Value*> args = {});
    void call_method_if_exists(const Variable& variable, const std::string& name, std::vector<llvm::Value*> args = {});

    void call_constructor(const Variable& variable, std::vector<llvm::Value*> args);
    void call_destructor(const Variable& variable);
    void destruct_all_variables(const Scope<Variable>& scope);

    void push_scope();
    Scope<Variable> pop_scope();

    void create_dua_init_function();
    void complete_dua_init_function();

private:

    llvm::LLVMContext context;
    llvm::Module module;
    llvm::IRBuilder<> builder;
    // Its insertion point is not guaranteed to be anywhere
    //  and thus, can be used at any point in time. This can
    //  be useful for example in inserting all the alloca
    //  instructions in the entry block of a function, regardless
    //  of their declaration location in the code.
    llvm::IRBuilder<> temp_builder;
    SymbolTable<Variable> symbol_table;
    std::unordered_map<std::string, FunctionInfo> functions;
    std::unordered_map<std::string, ClassType*> classes;
    // Nodes that are deferred to be evaluated after the evaluation
    //  of the whole tree. The nodes will be evaluated at the beginning
    //  of the entry point, in the order of insertions. This is useful
    //  for calling constructors of global objects for example, or for
    //  assigning them a non-constant value. All the deferred nodes will
    //  be evaluated inside a function called ".dua.init", which is called
    //  at the beginning of the main function.
    std::vector<ASTNode*> deferred_nodes;
    // Instead of having the fields be stored in the class type,
    //  and having them getting duplicated on each clone, let's
    //  keep the fields info in one place, and refer to it by name.
    std::unordered_map<std::string, std::vector<ClassField>> class_fields;
    std::unordered_map<std::string, std::vector<FieldConstructorArgs>> fields_args;
    std::unordered_map<std::string, llvm::Constant*> string_pool;
    llvm::Function* current_function = nullptr;
    llvm::StructType* current_class = nullptr;

    // Loops
    std::vector<llvm::BasicBlock*> continue_stack;
    std::vector<llvm::BasicBlock*> break_stack;

    std::string result;
};

}
