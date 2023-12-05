#pragma once

#include <string>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <types/Type.hpp>
#include <types/FunctionType.hpp>
#include <resolution/NameResolver.hpp>
#include <TypingSystem.hpp>
#include <Value.hpp>

namespace dua
{

class ASTNode;


class ModuleCompiler
{
public:

    friend class ASTNode;
    friend class ParserAssistant;
    friend class NameResolver;
    friend class FunctionNameResolver;
    friend class TypingSystem;
    friend class Value;
    friend class Type;
    friend class ClassType;

    ModuleCompiler(const std::string& module_name, const std::string& code);

    const std::string& get_result() { return result; }

    template <typename T, typename ...Args>
    T* create_node(Args ...args) {
        auto node = new T(this, args...);
        nodes.push_back(node);
        return node;
    }

    template <typename T, typename ...Args>
    const T* create_type(Args ...args) {
        return typing_system.create_type<T>(args...);
    }

    template <typename T>
    Value create_value(llvm::Value* value, const T* type) {
        return { &typing_system, value, type };
    }

    template <typename T, typename ...Args>
    Value create_value(llvm::Value* value, Args ...args) {
        return create_value(value, create_type(args...));
    }

    llvm::IRBuilder<>* get_builder() { return &builder; }
    llvm::LLVMContext* get_context() { return &context; }
    void push_deferred_node(ASTNode* node) { deferred_nodes.push_back(node); }

    // .dua.init function is a function that gets called before the entry point
    //  (at the beginning of the entry point), in which initializations and
    //  deferred nodes get executed.
    void create_dua_init_function();
    void complete_dua_init_function();

    ~ModuleCompiler();

private:

    // LLVM API for constructing IR
    llvm::LLVMContext context;
    llvm::Module module;
    llvm::IRBuilder<> builder;

    // Its insertion point is not guaranteed to be anywhere
    //  and thus, can be used at any point in time. This can
    //  be useful for example in inserting all the alloca
    //  instructions in the entry block of a function, regardless
    //  of their declaration location in the code.
    llvm::IRBuilder<> temp_builder;

    // Nodes that are deferred to be evaluated after the evaluation
    //  of the whole tree. The nodes will be evaluated at the beginning
    //  of the entry point, in the order of insertions. This is useful
    //  for calling constructors of global objects for example, or for
    //  assigning them a non-constant value. All the deferred nodes will
    //  be evaluated inside a function called ".dua.init", which is called
    //  at the beginning of the main function.
    std::vector<ASTNode*> deferred_nodes;

    // Used to avoid unnecessary allocations for same strings
    std::unordered_map<std::string, llvm::Constant*> string_pool;

    // The function being processed currently
    llvm::Function* current_function = nullptr;

    // The class being processed currently
    llvm::StructType* current_class = nullptr;

    // Loops
    std::vector<llvm::BasicBlock*> continue_stack;
    std::vector<llvm::BasicBlock*> break_stack;

    // Used to resolve names of identifiers, whether
    //  it's a variable or a function/method
    NameResolver name_resolver;

    // Used to create types, check for types, and convert between types
    TypingSystem typing_system;

    // Used to track allocated nodes to delete later
    std::vector<ASTNode*> nodes;

    // A cache of the resulting LLVM IR, used to
    //  avoid performing the same computations
    std::string result;
};

}
