#pragma once

#include <string>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <SymbolTable.h>
#include <types/TypeBase.h>
#include <types/ClassType.h>
#include "types/FunctionType.h"

namespace dua
{

class ASTNode;

struct FunctionInfo
{
    FunctionType type;
    std::vector<std::string> param_names;
};

struct Variable
{
    llvm::Value* ptr;
    TypeBase* type;
};

class ModuleCompiler
{
public:

    friend class ASTNode;
    friend class ParserAssistant;

    ModuleCompiler(const std::string& module_name, const std::string& code);

    // Returns the result type of an operation involving the two types.
    TypeBase* get_winning_type(TypeBase* lhs, TypeBase* rhs);

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

    void register_function(std::string name, FunctionInfo signature);
    FunctionInfo& get_function(const std::string& name);

    llvm::IRBuilder<>* get_builder() { return &builder; }
    llvm::LLVMContext* get_context() { return &context; }
    auto get_class(const std::string& name) { return classes[name]; }
    auto& get_class_fields() { return class_fields; }

    bool has_function(const std::string& name) const;
    void define_function_alias(const std::string& from, const std::string& to);
    llvm::CallInst* call_function(const std::string& name, std::vector<llvm::Value*> args = {});
    void call_method_if_exists(const Variable& variable, const std::string& name);
    void destruct_all_variables(const Scope<Variable>& scope);

    void push_scope();
    Scope<Variable> pop_scope();

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
    SymbolTable<std::string> function_aliases;
    std::unordered_map<std::string, FunctionInfo> functions;
    std::unordered_map<std::string, ClassType*> classes;
    // Nodes that are deferred to be evaluated after the evaluation
    //  of the whole tree. The nodes will be evaluated at the beginning
    //  of the entry point, in the order of insertions. This is useful
    //  for calling constructors of global objects for example.
    std::vector<ASTNode*> deferred_nodes;
    // Instead of having the fields be stored in the class type,
    //  and having them getting duplicated on each clone, let's
    //  keep the fields info in one place, and refer to it by name.
    std::unordered_map<std::string, std::vector<ClassField>> class_fields;
    std::unordered_map<std::string, llvm::Constant*> string_pool;
    llvm::Function* current_function = nullptr;
    llvm::StructType* current_class = nullptr;


    // Loops
    std::vector<llvm::BasicBlock*> continue_stack;
    std::vector<llvm::BasicBlock*> break_stack;

    std::string result;
};

}
