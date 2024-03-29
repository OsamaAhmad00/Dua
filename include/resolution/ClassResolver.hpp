#pragma once

#include <resolution/CommonStructs.hpp>
#include <map>
#include "Value.hpp"

namespace dua
{

class ClassType;
class ClassField;
class VTable;
class TypeAliasNode;

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
    // The number of fields introduced newly in a class
    std::unordered_map<std::string, size_t> owned_fields_count;

    Value class_names_array;

    // Stores the address of the global instance of the vtable for each class
    std::unordered_map<std::string, VTable*> vtables;

    std::unordered_map<std::string, std::vector<TypeAliasNode*>> class_aliases;

    const ClassType* get_class(const std::string& name);
    bool has_class(const std::string& name);
    void add_fields_constructor_args(std::string constructor_name, std::vector<FieldConstructorArgs> args);
    void construct_class_fields(const std::string &name, ClassDefinitionNode* node);
    std::vector<FieldConstructorArgs>& get_fields_args(const std::string& constructor_name);
    void create_vtable(const std::string& class_name);
    VTable* get_vtable_instance(const std::string& class_name);
    const Type* get_vtable_type(const std::string& class_name);
    ClassField get_vtable_field(const std::string& class_name);
    std::vector<NamedFunctionValue> get_all_class_methods(const std::string& class_name);
    std::vector<TypeAliasNode*>& get_class_aliases(const std::string& class_name);

    virtual ~ClassResolver();
};

struct VTable
{
    // 1 - The class name pointer
    // 2 - The parent pointer
    static constexpr int RESERVED_FIELDS_COUNT = 2;

    const ClassType* owner;
    llvm::GlobalVariable* instance;
    llvm::Type* llvm_type;

    // A map of the method name to its index in the vtable
    std::unordered_map<std::string, size_t> method_indices;
    // The name without the class prefix and without
    //  the self parameter. This is for easier searching
    //  for methods, without the concern of the owner class.
    std::map<std::string, std::string> method_names_without_class_prefix;

    llvm::Value* get_ith_element(size_t i, llvm::Type* type, llvm::Value* instance = nullptr);
    llvm::Value* get_method(const std::string& name, llvm::Type* type, llvm::Value* instance = nullptr);
    bool has_method(const std::string& name) const;

};

}