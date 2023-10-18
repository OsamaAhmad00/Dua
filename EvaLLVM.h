#pragma once

#include <memory>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

class EvaLLVM {

public:

    explicit EvaLLVM(const std::string& name) {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>(name, *context);
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
    }

    void eval(const std::string& code, const std::string& outfile = "out.ll") {

        // Parse

        // Generate IR
        compile(/* AST */);

        // Output
        module->print(llvm::outs(), nullptr);  // Print
        save_module(outfile);
    }

private:

    void compile(/* AST */) {
        llvm::Function* function = create_function("main");
        construct_function_body(function);
    }

    void construct_function_body(llvm::Function* function) {
        // For now, it has only one basic block.
        builder->SetInsertPoint(&function->back());

        llvm::Value* ret = builder->getInt32(42);

        ret = builder->CreateIntCast(ret, builder->getInt32Ty(), true);

        builder->CreateRet(ret);
    }

    llvm::Function* create_function(const std::string& name) {
        llvm::Function* function = module->getFunction("main");

        if (!function) {
            llvm::FunctionType* type = llvm::FunctionType::get(builder->getInt32Ty(), false);
            function = create_function_prototype(name, type);
        }

        attach_function_entry_block(function);

        return function;
    }

    llvm::Function* create_function_prototype(const std::string& name, llvm::FunctionType* type) {
        llvm::Function* function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, *module);
        llvm::verifyFunction(*function);
        return function;
    }

    llvm::BasicBlock* attach_function_entry_block(llvm::Function* function) {
        return llvm::BasicBlock::Create(*context, "entry", function);
    }

    void save_module(const std::string& outfile) {
        std::error_code error;
        llvm::raw_fd_ostream out(outfile, error);
        module->print(out, nullptr);
    }

    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
};


