#pragma once

#include <memory>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include "Expression.h"
#include "SymbolTable.h"

using Parameters = std::vector<std::pair<std::string, std::string>>;

namespace syntax {
    class Parser;
}

class Compiler {

public:

    explicit Compiler(const std::string& name);
    ~Compiler();
    void compile(const std::string& code, const std::string& outfile = "out.ll");

private:

    llvm::Value* eval(const Expression& expression);
    llvm::GlobalVariable* create_global_variable(const std::string& name, llvm::Type* type, const Expression& init_exp);
    llvm::AllocaInst* create_local_variable(const std::string& name, llvm::Type* type, llvm::Value* init=nullptr);
    llvm::LoadInst* get_global_variable(const std::string& name);
    llvm::LoadInst* get_local_variable(const std::string& name);
    llvm::CallInst* call_function(const std::string& name, const std::vector<llvm::Value*>& args);
    llvm::Function* define_function(const std::string& name, const Expression& body, const std::string& return_type="void", const Parameters& parameters={}, bool is_var_arg=false);
    llvm::Function* declare_function(const std::string& name, const std::string& return_type="void", const Parameters& parameters={}, bool is_var_arg=false);
    llvm::BasicBlock* create_basic_block(const std::string& name, llvm::Function* function);
    llvm::Constant* get_constant(const Expression& expression);
    llvm::Constant* create_string_literal(const std::string& name, const std::string& str);
    llvm::ConstantInt* create_integer_literal(long long num);
    llvm::Type* get_type(const std::string& str, bool panic_if_invalid=true);
    void init_external_references();
    void init_primitive_types();
    void save_module(const std::string& outfile);

    // Operations
    llvm::Value* call_printf(const Expression& expression);
    llvm::Value* eval_function(const Expression& expression);
    llvm::Value* eval_return(const Expression& expression);
    llvm::Value* eval_function_call(const Expression& expression);
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
    llvm::Value* eval_if(const Expression& expression);
    llvm::Value* eval_while(const Expression& expression);

    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    // Its insertion point is not guaranteed to be anywhere
    //  and thus, can be used at any point in time. This can
    //  be useful for example in inserting all the alloca
    //  instructions in the entry block of a function, regardless
    //  of their declaration location in the code.
    std::unique_ptr<llvm::IRBuilder<>> temp_builder;

    SymbolTable<llvm::AllocaInst*, llvm::GlobalVariable*> symbol_table;
    std::unordered_map<std::string, llvm::Type*> types;

    // FIXME
    //  The parser generator generates the definition of the classes and functions
    //  that it uses in the header file, thus you can include it only once across
    //  all files that will be part of the compilation, otherwise, you will have
    //  the classes defined more than once, resulting in linking errors. To overcome
    //  this, the parser is only included in the compiler source file. Since we can't
    //  include it here, we can't use a unique pointer for it because unique pointers
    //  need to know the size of the class to preform static assertions. Another possible
    //  solution would be to mark every definition in the header file as static, yet,
    //  this solution will duplicate the code of the parser in every compilation unit
    //  that uses it, which would be a big waste of space if it's included frequently.
    syntax::Parser* parser;

    llvm::Function* current_function;
};

