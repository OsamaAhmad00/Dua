#pragma once

#include <string>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <SymbolTable.h>

class ModuleCompiler
{
public:

    friend class ASTNode;

    ModuleCompiler(const std::string& module_name, const std::string& code);

    const std::string& get_result() { return result; }

    template <typename T, typename ...Args>
    T* create_node(Args ...args)
    {
        T* node = new T(args...);
        node->module_compiler = this;
        return node;
    }

    struct Variable {
        llvm::Value* ptr;
        llvm::Type* type;
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
    llvm::Function* current_function;

    std::string result;
};

