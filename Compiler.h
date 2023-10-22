#pragma once

#include <memory>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include "Expression.h"
#include "SymbolTable.h"

namespace syntax {
    class Parser;
}

class Compiler {

public:

    explicit Compiler(const std::string& name);
    ~Compiler();
    void compile(const std::string& code, const std::string& outfile = "out.ll");

private:

    llvm::GlobalVariable* create_global_variable(const std::string& name, llvm::Constant* initializer);
    llvm::AllocaInst* create_local_variable(const std::string& name, llvm::Constant* initializer);
    llvm::LoadInst* get_global_variable(const std::string& name);
    llvm::LoadInst* get_local_variable(const std::string& name);
    llvm::Constant* create_string_literal(const std::string& name, const std::string& str);
    llvm::ConstantInt* create_integer_literal(long long num);
    llvm::CallInst* call_function(const std::string& name, const std::vector<llvm::Value*>& args);
    llvm::Value* eval(const Expression& expression);
    void construct_function_body(llvm::Function* function, Expression& expression);
    void eval_main(Expression& expression);
    llvm::Function* create_function(const std::string& name);
    llvm::Function* create_function_prototype(const std::string& name, llvm::FunctionType* type);
    llvm::BasicBlock* attach_function_entry_block(llvm::Function* function);
    void save_module(const std::string& outfile);
    void init_external_references();
    llvm::Constant* get_expression_value(const Expression& expression);

    llvm::Value* call_printf(const Expression& expression);
    llvm::Value* eval_scope(const Expression& expression);
    llvm::AllocaInst* create_local_variable(const Expression& expression);
    llvm::GlobalVariable* create_global_variable(const Expression& expression);
    llvm::Value* set_variable(const Expression& expression);
    llvm::Value* eval_sum(const Expression& expression);
    llvm::Value* eval_sub(const Expression& expression);
    llvm::Value* eval_mul(const Expression& expression);
    llvm::Value* eval_div(const Expression& expression);
    llvm::Value* eval_less_than(const Expression& expression);
    llvm::Value* eval_greater_than(const Expression& expression);
    llvm::Value* eval_less_than_eq(const Expression& expression);
    llvm::Value* eval_greater_than_eq(const Expression& expression);
    llvm::Value* eval_equal(const Expression& expression);
    llvm::Value* eval_not_equal(const Expression& expression);

    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    SymbolTable<llvm::AllocaInst*, llvm::GlobalVariable*> symbol_table;

    // FIXME
    //  The parser generator generates the definition of the classes and functions
    //  that it uses in the header file, thus you can include it only once across
    //  all files that will be part of the compilation, otherwise, you will have
    //  the classes defined more than once, resulting in linking errors. To overcome
    //  this, the parser is only included in the compiler source file. Since we can't
    //  include it here, we can't use a unique pointer for it since the unique pointer
    //  needs to know the size of the class to do static assertions. A solution would
    //  be to mark every definition in the header file as static.
    syntax::Parser* parser;
};

