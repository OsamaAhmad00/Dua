#pragma once

#include <resolution/CommonStructs.hpp>

namespace dua
{

class ClassType;
class ClassField;

class ClassResolver
{
    ModuleCompiler* compiler;

public:

    ClassResolver(ModuleCompiler* compiler) : compiler(compiler) {}

    std::unordered_map<std::string, const ClassType*> classes;
    // Instead of having the fields be stored in the class type,
    //  and having them getting duplicated on each clone, let's
    //  keep the fields info in one place, and refer to it by name.
    std::unordered_map<std::string, std::vector<ClassField>> class_fields;
    std::unordered_map<std::string, std::vector<FieldConstructorArgs>> fields_args;

    const ClassType* get_class(const std::string& name);
    bool has_class(const std::string& name);
    void add_fields_constructor_args(std::string constructor_name, std::vector<FieldConstructorArgs> args);
    std::vector<FieldConstructorArgs>& get_fields_args(const std::string& constructor_name);
};

}