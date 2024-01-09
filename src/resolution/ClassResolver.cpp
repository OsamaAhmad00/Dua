#include <resolution/ClassResolver.hpp>
#include <utils/ErrorReporting.hpp>
#include <types/ClassType.hpp>
#include <Value.hpp>
#include <ModuleCompiler.hpp>
#include "types/PointerType.hpp"
#include "types/ArrayType.hpp"
#include "types/IntegerTypes.hpp"
#include "AST/values/StringValueNode.hpp"

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

void ClassResolver::create_class_names_array()
{
    std::vector<llvm::Constant*> array_content(class_id.size());
    for (auto& [name, id] : class_id) {
        array_content[id] = compiler->create_string(name + "_name", name).get_constant();
    }

    auto type = compiler->create_type<ArrayType>(
        compiler->create_type<PointerType>(
            compiler->create_type<I8Type>()
        ),
        array_content.size()
    );

    auto llvm_type = type->llvm_type();

    llvm::Constant* array_initializer = llvm::ConstantArray::get(llvm_type, std::move(array_content));

    compiler->get_module()->getOrInsertGlobal(".class_names", llvm_type);
    llvm::GlobalVariable* array = compiler->get_module()->getGlobalVariable(".class_names");
    array->setInitializer(array_initializer);
    array->setConstant(true);

    class_names_array.set(array);
    class_names_array.type = type;
}

Value ClassResolver::get_class_name(Value id)
{
    if (class_names_array.is_null())
        report_internal_error("Can't resolve names of classes dynamically before the registration of all classes");

    auto& builder = *compiler->get_builder();
    auto zero = builder.getInt64(0);
    auto array_type = class_names_array.type->as<ArrayType>();
    auto element_type = array_type->get_element_type();
    auto indices = std::vector<llvm::Value*> { zero, id.get() };
    auto ptr = builder.CreateGEP(array_type->llvm_type(), class_names_array.get(), indices);
    auto name = builder.CreateLoad(element_type->llvm_type(), ptr);

    return compiler->create_value(name, element_type);
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