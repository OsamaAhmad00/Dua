#pragma once

#include <string>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <types/Type.hpp>
#include <resolution/NameResolver.hpp>
#include <TypingSystem.hpp>
#include <Value.hpp>
#include <resolution/TemplatedNameResolver.hpp>

namespace dua
{

class ASTNode;
class ParserAssistant;

class ModuleCompiler
{
public:

    friend class ASTNode;
    friend class ParserAssistant;
    friend class NameResolver;
    friend class FunctionNameResolver;
    friend class ClassResolver;
    friend class TemplatedNameResolver;
    friend class TypingSystem;
    friend class Value;
    friend class Type;
    friend class ClassType;
    friend class ParserFacade;

    ModuleCompiler(const std::string& module_name, std::string code, bool include_libdua = true);

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

    Value create_value(llvm::Value* value, const Type* type, llvm::Value* memory_location = nullptr) {
        return { &typing_system, value, type, memory_location };
    }

    Value create_value(const Type* type, llvm::Value* memory_location) {
        return create_value(nullptr, type, memory_location);
    }

    llvm::IRBuilder<>* get_builder() { return &builder; }
    llvm::LLVMContext* get_context() { return &context; }
    llvm::Module* get_module() { return &module; }
    NameResolver& get_name_resolver() { return name_resolver; }
    TypingSystem& get_typing_system() { return typing_system; }
    void push_deferred_node(ASTNode* node) { deferred_nodes.push_back(node); }

    void create_the_object_class();

    void push_scope();
    Scope<Value> pop_scope();

    // Used in return expressions, break, continue, end of blocks,
    //  and any construct that needs to destruct the current scope,
    //  or the whole scope of a function, without popping it. These
    //  functions also check whether the current insertion basic block
    //  is terminated or not (has a branch or return instruction)
    //  before destructing. If so, they don't call the destructors,
    //  assuming that the terminal instruction has handled the destruction.
    //  As an example, if there is a return statement at the end of a block,
    //  both will try to destruct the variables at the current scope (the
    //  return statement will destruct the whole function scope), yet,
    //  the block node will find that there is a return instruction,
    //  and will assume that it has handled the destruction.
    void destruct_last_scope();
    void destruct_function_scope();  // Used mainly in return statements
    void destruct_global_scope();

    void push_scope_counter();
    void pop_scope_counter();

    // .dua.init function is a function that gets called at the startup before
    //  the entry point, in which initializations and deferred nodes get executed.
    void create_dua_init_function();
    void complete_dua_init_function();
    llvm::Function* get_dua_init_function();

    // Just like .dua.init, but at the end of the program
    void create_dua_cleanup_function();
    void complete_dua_cleanup_function();
    llvm::Function* get_dua_cleanup_function();

    // A function used when performing dynamic casting
    void create_dynamic_casting_function();
    void delete_dynamic_casting_function();

    Value create_string(const std::string& name, const std::string& value);

    std::string get_current_status();
    void report_error(const std::string& message);
    void report_internal_error(const std::string& message);
    void report_warning(const std::string& message);

    ~ModuleCompiler();

private:

    // LLVM API for constructing IR
    llvm::LLVMContext context;
    llvm::Module module;
    llvm::IRBuilder<> builder;

    ParserAssistant* parser_assistant;

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
    //  be evaluated inside a function called ".dua.init" (plus a .uuid
    //  postfix), which is called at the beginning of the main function.
    std::vector<ASTNode*> deferred_nodes;

    // Used to avoid unnecessary allocations for same strings
    std::map<std::string, llvm::Constant*> string_pool;

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

    // Used to keep track of the number of scopes used within a function, in
    //  order to determine how many scopes to destruct upon a return instruction
    std::vector<size_t> function_scope_count;

    // Each file will contain its own .dua.init function, and all .dua.init functions
    //  will be called at program startup. To differentiate between each function in
    //  each file, each .dua.init function gets appended with a uuid postfix. For this
    //  reason, the name of the .dua.init function should be stored to be able to
    //  retrieve it later.
    std::string dua_init_name;

    // Just like dua_init_name, but for the .dua.cleanup function
    std::string dua_cleanup_name;

    // A cache of the resulting LLVM IR, used to
    //  avoid performing the same computations
    std::string result;

public:

    // Mainly used when evaluating a templated nodes, in which the type of the children
    //  may change depending on the template arguments.
    bool stop_caching_types = false;

    // If true, nodes will clear their cache when getting their type
    bool clear_type_cache = false;

    // If this is true, the dua standard library will be included, and some syntactic sugars
    //  will be applied, such as creating a String class from string literals instead of i8*
    const bool include_libdua = true;
};

}
