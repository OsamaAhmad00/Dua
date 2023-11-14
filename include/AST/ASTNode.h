#pragma once

#include <ModuleCompiler.h>

#define STATE_MEMBER_GETTER(NAME) auto& NAME() { return compiler->NAME; }

// Just an indicator that the return
//  value is not going to be used.
using NoneValue = llvm::Value*;

class ASTNode
{
public:

    friend class ModuleCompiler;

    virtual ~ASTNode() = default;
    virtual llvm::Value* eval() = 0;

protected:

    llvm::BasicBlock* create_basic_block(const std::string& name, llvm::Function* function);
    llvm::AllocaInst* create_local_variable(const std::string& name, llvm::Type* type, llvm::Value* init);
    NoneValue none_value();

    // Debugging info
    // size_t line;
    // size_t column;

protected:

    ModuleCompiler* compiler = nullptr;

    // Convenience methods to access the internal state,
    // and also to ensure that they don't get set by accident.
    STATE_MEMBER_GETTER(context)
    STATE_MEMBER_GETTER(module)
    STATE_MEMBER_GETTER(builder)
    STATE_MEMBER_GETTER(temp_builder)
    STATE_MEMBER_GETTER(symbol_table)
    STATE_MEMBER_GETTER(current_function)
};
