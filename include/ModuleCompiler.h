#pragma once

#include <string>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <SymbolTable.h>
#include <types/TypeBase.h>
#include <FunctionInfo.h>

class ModuleCompiler
{
public:

    friend class ASTNode;

    ModuleCompiler(const std::string& module_name, const std::string& code);

    // Returns the result type of an operation involving the two types.
    TypeBase* get_winning_type(TypeBase* lhs, TypeBase* rhs);

    llvm::Value* cast_value(llvm::Value* value, llvm::Type* target_type);

    const std::string& get_result() { return result; }

    template <typename T, typename ...Args>
    T* create_node(Args ...args)
    {
        return new T(this, args...);
    }

    template <typename T, typename ...Args>
    T* create_type(Args ...args)
    {
        return new T(&builder, args...);
    }

    void register_function(std::string name, FunctionSignature signature);
    FunctionSignature& get_function(const std::string& name);

    llvm::IRBuilder<>* get_builder() { return &builder; }
    std::vector<llvm::BasicBlock*>& get_continue_stack() { return continue_stack; }
    std::vector<llvm::BasicBlock*>& get_break_stack() { return break_stack; }

    struct Variable {
        llvm::Value* ptr;
        TypeBase* type;
    };

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
    std::unordered_map<std::string, FunctionSignature> functions;
    llvm::Function* current_function;

    // Loops
    std::vector<llvm::BasicBlock*> continue_stack;
    std::vector<llvm::BasicBlock*> break_stack;

    std::string result;
};

