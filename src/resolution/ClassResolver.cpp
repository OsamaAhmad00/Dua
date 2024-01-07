#include <resolution/ClassResolver.hpp>
#include <utils/ErrorReporting.hpp>
#include <types/ClassType.hpp>
#include <Value.hpp>
#include <ModuleCompiler.hpp>
#include "types/PointerType.hpp"

namespace dua
{

void ClassResolver::add_fields_constructor_args(std::string constructor_name, std::vector<FieldConstructorArgs> args)
{
    fields_args[std::move(constructor_name)] = std::move(args);
}

std::vector<FieldConstructorArgs>& ClassResolver::get_fields_args(const std::string &constructor_name) {
    auto it = fields_args.find(constructor_name);
    if (it == fields_args.end()) {
        // This constructor has no arguments for fields, return an empty one
        return fields_args[""];
    }
    return it->second;
}

const ClassType *ClassResolver::get_class(const std::string &name) {
    auto it = classes.find(name);
    if (it == classes.end())
        report_internal_error("The class " + name + " is not defined");
    return it->second;
}

bool ClassResolver::has_class(const std::string &name) {
    return classes.find(name) != classes.end();
}

VTable* ClassResolver::get_vtable_instance(const std::string &class_name)
{
    auto it = vtables.find(class_name);
    if (it == vtables.end())
        report_internal_error("There is no vtable defined for the class " + class_name);
    return it->second;
}

ClassResolver::~ClassResolver() {
    for (const auto& vtable : vtables)
        delete vtable.second;
}

ClassField ClassResolver::get_vtable_field(const std::string &class_name) {
    auto instance = get_vtable_instance(class_name);
    return { ".vtable_ptr", get_vtable_type(class_name), instance->instance, {} };
}

void ClassResolver::create_vtable(const std::string &class_name)
{
    if (vtables.find(class_name) != vtables.end())
        return;

    auto methods = get_all_class_methods(class_name);

    // + 1 for the class id
    std::vector<llvm::Type*> body(methods.size() + 1);

    auto vtable_name = class_name + ".vtable";

    auto vtable_type = llvm::StructType::getTypeByName(*compiler->get_context(), vtable_name);
    if (vtable_type == nullptr)
        vtable_type = llvm::StructType::create(*compiler->get_context(), vtable_name);

    body[0] = compiler->get_builder()->getInt64Ty();
    for (size_t i = 1; i < body.size(); i++)
        body[i] = methods[i - 1].type->llvm_type()->getPointerTo();

    vtable_type->setBody(std::move(body));

    // + 1 for the class id
    std::vector<llvm::Constant*> content(methods.size() + 1);
    content[0] = compiler->get_builder()->getInt64(compiler->get_name_resolver().class_id[class_name]);
    for (size_t i = 1; i < content.size(); i++) {
        content[i] = llvm::dyn_cast<llvm::Constant>(methods[i - 1].ptr);
        assert(content[i] != nullptr);
    }
    auto value = llvm::ConstantStruct::get(vtable_type, std::move(content));

    auto instance_name = vtable_name + ".instance";
    compiler->get_module()->getOrInsertGlobal(instance_name, vtable_type);
    auto instance_ptr = compiler->get_module()->getGlobalVariable(instance_name);
    instance_ptr->setInitializer(value);

    auto class_type = compiler->get_name_resolver().get_class(class_name);

    auto instance = new VTable { class_type, instance_ptr, vtable_type };

    for (size_t i = 0; i < methods.size(); i++)
        instance->method_indices[methods[i].name] = i + 1;

    for (auto& method : methods)
        instance->method_names_without_class_prefix[method.name_without_class_prefix] = method.name;

    vtables[class_name] = instance;
}

const Type *ClassResolver::get_vtable_type(const std::string &class_name) {
    auto vtable_name = class_name + ".vtable";
    auto vtable_type = compiler->create_type<ClassType>(vtable_name);
    auto ptr_type = compiler->create_type<PointerType>(vtable_type);
    return ptr_type;
}

std::vector<NamedFunctionValue> ClassResolver::get_all_class_methods(const std::string &class_name)
{
    auto class_methods = compiler->get_name_resolver().get_class_methods(class_name, true);

    std::vector<NamedFunctionValue> methods;

    auto it = parent_classes.find(class_name);
    if (it == parent_classes.end()) {
        methods.resize(class_methods.size());
        for (size_t i = 0; i < methods.size(); i++) {
            methods[i] = class_methods[i];
            methods[i].name_without_class_prefix = methods[i].name.substr(class_name.size() + 1);
        }
        return methods;
    }

    auto parent = it->second;
    create_vtable(parent->name);
    auto parent_vtable = get_vtable_instance(parent->name);

    methods.resize(parent_vtable->method_indices.size());

    for (auto& method : class_methods)
    {
        auto self_param_position = method.name.find(class_name + "&");  // This must not be npos
        auto stripped_name = method.name.substr(class_name.size() + 1, self_param_position - (class_name.size() + 1 + 1));
        auto params_without_self = method.name.substr(self_param_position + class_name.size() + 1);
        auto parent_method_name = parent->name;
        parent_method_name += ".";
        parent_method_name += stripped_name;
        parent_method_name += ".";
        parent_method_name += parent->name;
        parent_method_name += "&";
        parent_method_name += params_without_self;
        method.name_without_class_prefix = method.name.substr(class_name.size() + 1);
        auto index = parent_vtable->method_indices.find(parent_method_name);
        if (index == parent_vtable->method_indices.end())
            methods.push_back(method);
        else
            methods[index->second - 1] = method;
    }

    // The indices here are the indices in the vtable, which include the class ID.
    for (auto& [name, index] : parent_vtable->method_indices) {
        if (!methods[index - 1].name.empty()) {
            // This method is overwritten by the current class implementation
            continue;
        }
        auto func = compiler->get_module()->getFunction(name);
        auto type = compiler->get_name_resolver().get_function_no_overloading(name).type;
        methods[index - 1] = { name, func, type, name.substr(parent->name.size() + 1) };
    }

    return methods;
}

llvm::Value* VTable::get_method(const std::string &name, llvm::Type* type, llvm::Value* instance)
{
    auto it = method_indices.find(name);
    if (it == method_indices.end())
        report_error("The class " + owner->name + " doesn't contain a method with the name " + name);

    size_t index = it->second;

    return get_ith_element(index, type, instance);
}

llvm::Value *VTable::get_ith_element(size_t i, llvm::Type* type, llvm::Value *instance)
{
    if (instance == nullptr)
        instance = this->instance;

    auto ptr = owner->compiler->get_builder()->CreateStructGEP(this->llvm_type, instance, i);
    auto value = owner->compiler->get_builder()->CreateLoad(type, ptr);

    return value;
}

}