#pragma once

#include <ModuleCompiler.hpp>
#include <types/Type.hpp>

namespace dua
{

#define STATE_MEMBER_GETTER(NAME) auto& NAME() const { return compiler->NAME; }

// Just an indicator that the return
//  value is not going to be used.
using NoneValue = llvm::Value*;

class ASTNode
{
public:

    friend class ModuleCompiler;

    virtual ~ASTNode() = default;

    virtual llvm::Value* eval() = 0;

    virtual const Type* get_type();

protected:

    llvm::BasicBlock* create_basic_block(const std::string& name, llvm::Function* function);

    llvm::AllocaInst* create_local_variable(const std::string& name, const Type* type, Value* init, std::vector<Value> args = {});

    NoneValue none_value();

    const Type* type = nullptr;
    ModuleCompiler* compiler = nullptr;

    // Convenience methods to access the internal state,
    // and also to ensure that they don't get set by accident.
    STATE_MEMBER_GETTER(context)
    STATE_MEMBER_GETTER(module)
    STATE_MEMBER_GETTER(builder)
    STATE_MEMBER_GETTER(temp_builder)
    STATE_MEMBER_GETTER(current_function)
    STATE_MEMBER_GETTER(current_class)
    STATE_MEMBER_GETTER(name_resolver)
    STATE_MEMBER_GETTER(string_pool)
    STATE_MEMBER_GETTER(continue_stack)
    STATE_MEMBER_GETTER(break_stack)
};

}
