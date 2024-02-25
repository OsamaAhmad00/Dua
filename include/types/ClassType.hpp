#pragma once

#include <types/Type.hpp>
#include <llvm/IR/DerivedTypes.h>

namespace dua
{

class Value;

class ASTNode;

struct ClassField
{
    std::string name;
    const Type* type;
    // Only one of the following can be present
    // X y = z -> z is the initializer
    // X x(y, z) -> y and z are the init args
    // An ASTNode is used here instead of a constant
    //  value to allow for binding of non-constant
    //  expressions to the class field
    ASTNode* initializer;
    std::vector<ASTNode*> init_args;
};

class ClassType : public Type
{

public:

    std::string name;
    // Methods are not stored in the class definition, rather, they're
    //  visible through the whole file, even before the class definition.
    //  In other words, class methods are treated just like regular functions.
    // We make use of the fact that the '.' character is not allowed to be in
    //  identifiers in the dua language, but allowed in LLVM IR. We prefix
    //  every function name with the name of the enclosing class, with a '.'
    //  in between (Printer.print for example). This way, we make sure that
    //  user defined function names can't collide with the method names.

    // The fields are stored and accessed through the compiler.

    ClassType(ModuleCompiler* compiler, std::string name);

    Value default_value() const override;

    Value zero_value() const override;

    llvm::StructType* llvm_type() const override;

    const std::vector<ClassField>& fields() const;

    std::string to_string() const override { return name; }

    std::string as_key() const override { return name; }

    const ClassField& get_field(const std::string& name) const;

    Value get_field(const Value& instance, const std::string& name) const;

    Value get_field(const Value& instance, size_t index) const;

    Value get_method(const std::string& name, Value instance, const std::vector<const Type*>& arg_types, bool panic_on_error = true) const;

    int ancestor_distance(const ClassType* ancestor) const;
};

}