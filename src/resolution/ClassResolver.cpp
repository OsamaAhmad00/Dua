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
        report_error("Redefinition of the class " + class_name);

    auto methods = compiler->get_name_resolver().get_class_methods(class_name);

    std::vector<llvm::Type*> body(methods.size());

    auto vtable_name = class_name + ".vtable";

    auto vtable_type = llvm::StructType::getTypeByName(*compiler->get_context(), vtable_name);
    if (vtable_type == nullptr)
        vtable_type = llvm::StructType::create(*compiler->get_context(), vtable_name);

    for (size_t i = 0; i < body.size(); i++)
        body[i] = methods[i].type->llvm_type()->getPointerTo();

    vtable_type->setBody(std::move(body));

    std::vector<llvm::Constant*> content(methods.size());
    for (size_t i = 0; i < content.size(); i++) {
        content[i] = llvm::dyn_cast<llvm::Constant>(methods[i].ptr);
        assert(content[i] != nullptr);
    }
    auto value = llvm::ConstantStruct::get(vtable_type, std::move(content));

    auto instance_name = vtable_name + ".instance";
    compiler->get_module()->getOrInsertGlobal(instance_name, vtable_type);
    auto instance_ptr = compiler->get_module()->getGlobalVariable(instance_name);
    instance_ptr->setInitializer(value);

    auto class_type = compiler->get_name_resolver().get_class(class_name);

    auto instance = new VTable { class_type, instance_ptr, vtable_type };

    for (size_t i = 0; i < methods.size(); i++) {
        instance->method_indices[methods[i].name] = i;
    }

    vtables[class_name] = instance;
}

const Type *ClassResolver::get_vtable_type(const std::string &class_name) {
    auto vtable_name = class_name + ".vtable";
    auto vtable_type = compiler->create_type<ClassType>(vtable_name);
    auto ptr_type = compiler->create_type<PointerType>(vtable_type);
    return ptr_type;
}

llvm::Value* VTable::get_method(const std::string &name, llvm::Type* type, llvm::Value* instance)
{
    auto it = method_indices.find(name);
    if (it == method_indices.end())
        report_error("The class " + owner->name + " doesn't contain a method with the name " + name);

    size_t index = it->second;

    if (instance == nullptr)
        instance = this->instance;

    auto method_ptr_ptr = owner->compiler->get_builder()->CreateStructGEP(this->llvm_type, instance, index);
    auto method_ptr = owner->compiler->get_builder()->CreateLoad(type, method_ptr_ptr);

    return method_ptr;
}

}