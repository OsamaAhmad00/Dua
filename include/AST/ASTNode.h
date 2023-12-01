#pragma once

#include <ModuleCompiler.h>
#include <types/Type.h>

namespace dua
{

#define STATE_MEMBER_GETTER(NAME) auto& NAME() { return compiler->NAME; }

// Just an indicator that the return
//  value is not going to be used.
using NoneValue = llvm::Value*;

class ASTNode
{
public:

    friend class ModuleCompiler;

    virtual ~ASTNode() { delete type; };
    virtual llvm::Value* eval() = 0;

    // The typing system
    Type* get_cached_type();
    virtual Type* compute_type();

protected:

    llvm::BasicBlock* create_basic_block(const std::string& name, llvm::Function* function);
    llvm::AllocaInst* create_local_variable(const std::string& name, Type* type, llvm::Value* init, std::vector<llvm::Value*> args = {});
    NoneValue none_value();

    Type* type = nullptr;
    ModuleCompiler* compiler = nullptr;

    // Convenience methods to access the internal state,
    // and also to ensure that they don't get set by accident.
    STATE_MEMBER_GETTER(context)
    STATE_MEMBER_GETTER(module)
    STATE_MEMBER_GETTER(builder)
    STATE_MEMBER_GETTER(temp_builder)
    STATE_MEMBER_GETTER(symbol_table)
    STATE_MEMBER_GETTER(functions)
    STATE_MEMBER_GETTER(current_function)
    STATE_MEMBER_GETTER(current_class)
    STATE_MEMBER_GETTER(string_pool)
    STATE_MEMBER_GETTER(classes)
    STATE_MEMBER_GETTER(continue_stack)
    STATE_MEMBER_GETTER(break_stack)
};

}
