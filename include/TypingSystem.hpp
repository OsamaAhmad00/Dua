#pragma once

#include <types/Type.hpp>
#include <llvm/IR/Type.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <SymbolTable.hpp>

namespace dua
{

class ModuleCompiler;
class Value;

class TypingSystem
{
    ModuleCompiler* compiler;
    mutable std::unordered_map<std::string, Type*> type_cache;

public:

    SymbolTable<const Type*> identifier_types;

    explicit TypingSystem(ModuleCompiler* compiler);

    template <typename T, typename ...Args>
    T* create_type(Args ...args) const
    {
        // This won't recurse deeply due to the nature of recursive types
        //  (like pointers), in which each layer (the type pointed to in
        //  the case of a pointer type) is created in this function.
        T t(compiler, args...);
        auto str = t.as_key();
        auto it = type_cache.find(str);
        T* result;
        result = ((it != type_cache.end()) ? (T*)it->second : new T(compiler, args...));
        type_cache[str] = result;
        return result;
    }

    // Returns 0 if the types are similar. The more the types are dissimilar, the more the score is.
    [[nodiscard]] int similarity_score(const Type* t1, const Type* t2) const;
    [[nodiscard]] int type_list_similarity_score(const std::vector<const Type*>& l1,
                             const std::vector<const Type*>& l2, bool can_differ_in_size = false) const;
    // Returns the result type of an operation involving the primitive two types.
    [[nodiscard]] const Type* get_winning_type(const Type* lhs, const Type* rhs, bool panic_on_failure=true, const std::string& message = "") const;
    [[nodiscard]] Value cast_value(const Value& value, const Type* target_type, bool panic_on_failure=true) const;
    [[nodiscard]] Value forced_cast_value(const Value& value, const Type *target_type) const;
    [[nodiscard]] Value cast_as_bool(const Value& value, bool panic_on_failure=true) const;
    [[nodiscard]] bool is_castable(const Type *t1, const Type* t2) const;

    [[nodiscard]] llvm::IRBuilder<>& builder() const;
    [[nodiscard]] llvm::Module& module() const;

    void insert_type(const std::string& name, const Type* type);
    void insert_global_type(const std::string& name, const Type* type);
    const Type* get_type(const std::string& name);

    void push_scope();
    Scope<const Type*> pop_scope();

    ~TypingSystem();
};

}