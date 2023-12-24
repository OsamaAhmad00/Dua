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
class ClassDefinitionNode;

struct TemplatedFunctionNode
{
    FunctionDefinitionNode* node;
    std::vector<std::string> template_params;
    FunctionInfo info;
    llvm::StructType* owner_class = nullptr;  // In case of a method
    bool in_templated_class = false;
};

struct TemplatedClassNode
{
    ClassDefinitionNode* node;
    std::vector<std::string> template_params;
};

struct TemplatedClassMethodInfo
{
    FunctionInfo info;
    // In case the method is templated as well
    std::vector<std::string> template_params;
};

struct TemplateBindings
{
    std::vector<std::string> params;
    std::vector<const Type*> args;
};

struct TemplatedClassFieldConstructorArgs
{
    FunctionDefinitionNode* func;
    std::vector<FieldConstructorArgs> args;
};

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

    Value create_value(llvm::Value* value, const Type* type, llvm::Value* memory_location = nullptr) {
        return { &typing_system, value, type, memory_location };
    }

    Value create_value(const Type* type, llvm::Value* memory_location) {
        return create_value(nullptr, type, memory_location);
    }

    llvm::IRBuilder<>* get_builder() { return &builder; }
    llvm::LLVMContext* get_context() { return &context; }
    NameResolver& get_name_resolver() { return name_resolver; }
    TypingSystem& get_typing_system() { return typing_system; }
    void push_deferred_node(ASTNode* node) { deferred_nodes.push_back(node); }

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

    void push_scope_counter();
    void pop_scope_counter();

    // .dua.init function is a function that gets called before the entry point
    //  (at the beginning of the entry point), in which initializations and
    //  deferred nodes get executed.
    void create_dua_init_function();
    void complete_dua_init_function();

    std::string get_templated_function_key(std::string name, size_t args_count);
    std::string get_templated_function_full_name(std::string name, const std::vector<const Type*>& template_args);
    std::string get_templated_function_full_name(std::string name, const std::vector<const Type*>& template_args, const std::vector<const Type*>& param_types);
    void add_templated_function(FunctionDefinitionNode* node, std::vector<std::string> template_params, FunctionInfo info, const std::string& class_name = "", bool in_templated_class = false);
    Value get_templated_function(const std::string& name, std::vector<const Type*>& template_args);
    Value get_templated_function(const std::string& name, std::vector<const Type*>& template_args, const std::vector<const Type*>& arg_types, bool use_arg_types = true);
    long long get_winner_templated_function(const std::string& name, const std::vector<TemplatedFunctionNode>& functions, const std::vector<const Type*>& template_args, const std::vector<const Type*>& arg_types, bool panic_on_not_found = true);

    std::string get_templated_class_key(std::string name, size_t args_count);
    std::string get_templated_class_full_name(const std::string& name, const std::vector<const Type*>& template_args);
    void add_templated_class(ClassDefinitionNode* node, std::vector<std::string> template_params);
    void add_templated_class_method_info(const std::string& cls, FunctionDefinitionNode* method, FunctionInfo info, std::vector<std::string> template_params);
    TemplatedClassMethodInfo get_templated_class_method_info(const std::string& cls, const std::string& method, const FunctionType* type, size_t template_param_count);
    const ClassType* get_templated_class(const std::string& name, const std::vector<const Type*>& template_args);
    void register_templated_class(const std::string& name, const std::vector<const Type*>& template_args);
    const ClassType* define_templated_class(const std::string& name, const std::vector<const Type*>& template_args);

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

    // A map of the templated functions, used to instantiate functions on demand
    std::unordered_map<std::string, std::vector<TemplatedFunctionNode>> templated_functions;

    // A map of the templated classes, used to instantiate classes on demand
    std::unordered_map<std::string, TemplatedClassNode> templated_classes;
    std::unordered_map<std::string, TemplatedClassMethodInfo> templated_class_method_info;
    std::unordered_map<std::string, TemplateBindings> templated_class_bindings;
    std::unordered_map<std::string, std::vector<TemplatedClassFieldConstructorArgs>> templated_class_field_constructor_args;

    // Mainly used when evaluating a templated nodes, in which the type of the children
    //  may change depending on the template arguments.
    bool stop_caching_types = false;

    // Used to determine the depth of the nested templated definitions
    size_t templated_definition_depth = 0;

    // A cache of the resulting LLVM IR, used to
    //  avoid performing the same computations
    std::string result;
};

}
