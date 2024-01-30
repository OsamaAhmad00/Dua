#pragma once

#include <types/Type.hpp>
#include <utils/ErrorReporting.hpp>

namespace dua
{

struct NullType : Type
{
    NullType(ModuleCompiler* compiler) { this->compiler = compiler; }

    Value default_value() const override {
        compiler->report_internal_error("Null types have no value");
        return {};
    }

    llvm::Type* llvm_type() const override {
        // Null types has no value, and no variable can hold the
        //  null type. This is only used to represent null pointers.
        // LLVM-wise, the type of a null pointer is just a "ptr" type,
        //  and it doesn't matter what type it points to since it's
        //  just an opaque pointer. Thus, we can return any type that
        //  can be pointed to here.
        return compiler->get_builder()->getInt1Ty();
    }

    std::string to_string() const override { return "Null"; }

    std::string as_key() const override { return "Null"; }
};

}
