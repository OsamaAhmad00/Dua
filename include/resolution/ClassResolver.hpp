#pragma once

#include <resolution/CommonStructs.hpp>
#include "Value.hpp"

namespace dua
{

class ClassType;
class ClassField;
class VTable;

class ClassResolver
{
    ModuleCompiler* compiler;

public:

    ClassResolver(ModuleCompiler* compiler) : compiler(compiler) {}

    std::unordered_map<std::string, const ClassType*> classes;
    // TODO move these maps into the class type
    std::unordered_map<std::string, std::vector<ClassField>> class_fields;
    std::unordered_map<std::string, std::vector<FieldConstructorArgs>> fields_args;
    std::unordered_map<std::string, const ClassType*> parent_classes;

    // Stores the address of the global instance of the vtable for each class
    std::unordered_map<std::string, VTable*> vtables;

    const ClassType* get_class(const std::string& name);
    bool has_class(const std::string& name);
    void add_fields_constructor_args(std::string constructor_name, std::vector<FieldConstructorArgs> args);
    std::vector<FieldConstructorArgs>& get_fields_args(const std::string& constructor_name);
    void create_vtable(const std::string& class_name);
    VTable* get_vtable_instance(const std::string& class_name);
    const Type* get_vtable_type(const std::string& class_name);
    ClassField get_vtable_field(const std::string& class_name);
    std::vector<NamedFunctionValue> get_all_class_methods(const std::string& class_name);

    virtual ~ClassResolver();
};

struct VTable
{
    const ClassType* owner;
    llvm::GlobalVariable* instance;
    llvm::Type* llvm_type;

    // A map of the method name to its index in the vtable
    std::unordered_map<std::string, size_t> method_indices;

    llvm::Value* get_method(const std::string& name, llvm::Type* type, llvm::Value* instance = nullptr);
};

}