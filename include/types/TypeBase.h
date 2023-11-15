#pragma once

#include <llvm/IR/Constant.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/IRBuilder.h>

struct TypeBase
{
    // FIXME the lifetime of the a type is the same as
    //  the lifetime of the ValueNode encompassing it.
    //  This leads to the necessity to either copy the
    //  type if it's going to live past the value node,
    //  or make sure that the value node lives long enough.
    //  This is a candidate for int introducing bugs.
    llvm::IRBuilder<>* builder;
    virtual llvm::Constant* default_value() = 0;
    virtual llvm::Type* llvm_type() = 0;
    virtual TypeBase* clone() = 0;
    virtual ~TypeBase() = default;

    llvm::Type* operator->() { return llvm_type(); }
    operator llvm::Type*() { return llvm_type(); }
};