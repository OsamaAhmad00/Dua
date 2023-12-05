#pragma once

#include <llvm/IR/Constant.h>
#include <llvm/IR/Type.h>

namespace dua
{

class ModuleCompiler;

struct Type
{
    ModuleCompiler* compiler;
    virtual llvm::Constant* default_value() const = 0;
    virtual llvm::Type* llvm_type() const = 0;
    virtual std::string to_string() const = 0;
    virtual std::string as_key() const;
    virtual bool operator==(const Type& other);
    virtual bool operator!=(const Type& other);
    virtual ~Type() = default;

    llvm::Type* operator->() const;
    operator llvm::Type*() const;

    // Delegates to TypingSystem
    [[nodiscard]] const Type* get_winning_type(const Type* other, bool panic_on_failure=true) const;
    [[nodiscard]] bool is_castable(const Type* type) const;
};

}