#pragma once

#include <types/FunctionType.hpp>
#include <vector>

namespace dua
{

class ASTNode;
class FunctionDefinitionNode;
class ClassDefinitionNode;
class IdentifierType;

struct FunctionInfo
{
    const FunctionType* type;
    std::vector<std::string> param_names;
    bool is_templated = false;
};

struct FieldConstructorArgs
{
    std::string name;
    std::vector<ASTNode*> args;
};

struct TemplatedFunctionNode
{
    FunctionDefinitionNode* node;
    std::vector<std::string> template_params;
    FunctionInfo info;
    llvm::StructType* owner_class = nullptr;  // In case of a method
    bool in_templated_class = false;
};

struct ParentClassInfo
{
    std::string name;
    bool is_templated;
    std::vector<const Type*> template_args;
};

struct TemplatedClassNode
{
    ClassDefinitionNode* node;
    std::vector<std::string> template_params;
    const IdentifierType* parent;
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

struct NamedFunctionValue
{
    std::string name;
    llvm::Function* ptr;
    const Type* type;
    std::string name_without_class_prefix;
};

}
