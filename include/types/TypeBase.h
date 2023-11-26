#pragma once

#include <llvm/IR/Constant.h>
#include <llvm/IR/Type.h>

namespace dua
{

class ModuleCompiler;

struct TypeBase
{
    // FIXME the lifetime of the a type is the same as
    //  the lifetime of the ValueNode encompassing it.
    //  This leads to the necessity to either copy the
    //  type if it's going to live past the value node,
    //  or make sure that the value node lives long enough.
    //  This is a candidate for int introducing bugs.
    ModuleCompiler* compiler;
    virtual llvm::Constant* default_value() = 0;
    virtual llvm::Type* llvm_type() const = 0;
    virtual TypeBase* clone() = 0;
    virtual bool operator==(const TypeBase& other) { return llvm_type() == other.llvm_type(); }
    virtual bool operator!=(const TypeBase& other) { return !(*this == other); }
    virtual ~TypeBase() = default;

    llvm::Type* operator->() { return llvm_type(); }
    operator llvm::Type*() { return llvm_type(); }
};

}