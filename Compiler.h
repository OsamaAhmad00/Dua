#pragma once

#include <memory>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include "Expression.h"

namespace syntax {
    class Parser;
}

template <typename T>
struct UniquePointer
{
    // A unique pointer with no static assertions that
    // need to know the size of the objects pointed to.
    T* ptr;
    template <typename ...ArgsType>
    UniquePointer(ArgsType... args) { ptr = new T(args...); }
    ~UniquePointer() { delete ptr; }
    T* operator->() { return ptr; }
    T& operator*() { return *ptr; }
};

class Compiler {

public:

    explicit Compiler(const std::string& name);
    void compile(const std::string& code, const std::string& outfile = "out.ll");

private:

    llvm::Constant* create_string_literal(const std::string& name, const std::string& str);
    llvm::ConstantInt* create_integer_literal(long long num);
    llvm::CallInst* call_function(const std::string& name, const std::vector<llvm::Value*>& args);
    llvm::Value* eval(Expression& expression);
    void construct_function_body(llvm::Function* function, Expression& expression);
    void eval_main(Expression& expression);
    llvm::Function* create_function(const std::string& name);
    llvm::Function* create_function_prototype(const std::string& name, llvm::FunctionType* type);
    llvm::BasicBlock* attach_function_entry_block(llvm::Function* function);
    void save_module(const std::string& outfile);
    void init_external_references();

    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    UniquePointer<syntax::Parser> parser;
};

