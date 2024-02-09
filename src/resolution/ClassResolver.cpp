#include <resolution/ClassResolver.hpp>
#include <utils/ErrorReporting.hpp>
#include <types/ClassType.hpp>
#include <Value.hpp>
#include <ModuleCompiler.hpp>
#include "types/PointerType.hpp"
#include "types/IntegerTypes.hpp"
#include "AST/values/StringValueNode.hpp"
#include <AST/class/ClassDefinitionNode.hpp>
#include <AST/types/TypeAliasNode.hpp>

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
        compiler->report_internal_error("The class " + name + " is not defined");
    return it->second;
}

bool ClassResolver::has_class(const std::string &name) {
    return classes.find(name) != classes.end();
}

VTable* ClassResolver::get_vtable_instance(const std::string &class_name)
{
    auto it = vtables.find(class_name);
    if (it == vtables.end())
        compiler->report_internal_error("There is no vtable defined for the class " + class_name);
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

    // + 2 for the class name pointer and the parent pointer
    std::vector<llvm::Type*> body(methods.size() + VTable::RESERVED_FIELDS_COUNT);

    auto vtable_name = class_name + ".vtable";

    auto vtable_type = llvm::StructType::getTypeByName(*compiler->get_context(), vtable_name);
    if (vtable_type == nullptr)
        vtable_type = llvm::StructType::create(*compiler->get_context(), vtable_name);

    auto name_str = compiler->create_string(class_name + "_name", class_name);

    auto parent_vtable = (class_name == "Object") ? llvm::Constant::getNullValue(vtable_type->getPointerTo())
            : get_vtable_instance(parent_classes[class_name]->name)->instance;

    body[0] = name_str.type->llvm_type();
    body[1] = parent_vtable->getType();
    for (size_t i = VTable::RESERVED_FIELDS_COUNT; i < body.size(); i++)
        body[i] = methods[i - VTable::RESERVED_FIELDS_COUNT].type->llvm_type()->getPointerTo();

    vtable_type->setBody(std::move(body));

    // + 2 for the class name pointer and the parent pointer
    std::vector<llvm::Constant*> content(methods.size() + VTable::RESERVED_FIELDS_COUNT);
    content[0] = name_str.get_constant();
    content[1] = parent_vtable;
    for (size_t i = VTable::RESERVED_FIELDS_COUNT; i < content.size(); i++) {
        content[i] = llvm::dyn_cast<llvm::Constant>(methods[i - VTable::RESERVED_FIELDS_COUNT].ptr);
        assert(content[i] != nullptr);
    }
    auto value = llvm::ConstantStruct::get(vtable_type, std::move(content));

    auto instance_name = vtable_name + ".instance";
    compiler->get_module()->getOrInsertGlobal(instance_name, vtable_type);
    auto instance_ptr = compiler->get_module()->getGlobalVariable(instance_name);
    instance_ptr->setInitializer(value);

    auto comdat = compiler->get_module()->getOrInsertComdat(instance_name);
    comdat->setSelectionKind(llvm::Comdat::Any);
    instance_ptr->setComdat(comdat);

    auto class_type = compiler->get_name_resolver().get_class(class_name);

    auto instance = new VTable { class_type, instance_ptr, vtable_type };

    for (size_t i = 0; i < methods.size(); i++)
        instance->method_indices[methods[i].name] = i + VTable::RESERVED_FIELDS_COUNT;

    for (auto& method : methods)
        instance->method_names_without_class_prefix[method.name_without_class_prefix] = method.name;

    vtables[class_name] = instance;
}

void ClassResolver::construct_class_fields(const std::string &name, ClassDefinitionNode* node)
{
    auto class_type = compiler->name_resolver.get_class(name)->llvm_type();

    auto parent = compiler->name_resolver.parent_classes[name];
    bool is_packed;
    if (parent->name == "Object") {
        is_packed = node->is_packed;
    }  else {
        auto parent_llvm = llvm::StructType::getTypeByName(compiler->context, parent->name);
        is_packed = parent_llvm->isPacked();
    }

    compiler->typing_system.identifier_types.keep_only_last_n_scopes(0, true);

    compiler->typing_system.push_scope();
    for (auto alias : node->aliases)
        alias->eval();

    compiler->name_resolver.create_vtable(name);

    auto& fields = compiler->name_resolver.class_fields[name];

    compiler->name_resolver.owned_fields_count[name] = fields.size();

    auto& parent_fields = compiler->name_resolver.class_fields[parent->name];

    // Ignore the vtable field
    for (size_t i = 1; i < parent_fields.size(); i++)
        for (auto &field : fields)
            if (parent_fields[i].name == field.name)
                compiler->report_error("The field " + parent_fields[i].name + " of the class " + name + " is already defined one of its parent classes");

    std::vector<ClassField> all_fields;
    // We need to exclude the vtable field of the parent
    all_fields.reserve(1 + fields.size() + parent_fields.size() - 1);

    all_fields.push_back(compiler->name_resolver.get_vtable_field(name));

    for (size_t i = 1; i < parent_fields.size(); i++)
        all_fields.push_back(parent_fields[i]);

    for (auto& field : fields)
        all_fields.push_back(field);

    fields = std::move(all_fields);

    for (auto& field : fields)
        field.type = field.type->get_concrete_type();

    std::vector<llvm::Type*> body(fields.size());
    for (size_t i = 0; i < body.size(); i++) {
        body[i] = fields[i].type->llvm_type();
    }

    class_type->setBody(std::move(body), is_packed);

    compiler->typing_system.identifier_types.restore_prev_state();
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
        // This is the object class only
        assert(class_name == "Object");
        methods.resize(class_methods.size());
        for (size_t i = 0; i < methods.size(); i++) {
            methods[i] = class_methods[i];
            auto self_param_position = methods[i].name.find(class_name + "&");  // This must not be npos
            auto stripped_name = methods[i].name.substr(class_name.size() + 1, self_param_position - (class_name.size() + 1 + 1));
            auto params_without_self = methods[i].name.substr(self_param_position + class_name.size() + 1);
            methods[i].name_without_class_prefix = std::move(stripped_name);
            methods[i].name_without_class_prefix += '(' + ((params_without_self != ")") ? params_without_self.substr(2) : ")");
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
        parent_method_name += "(";
        parent_method_name += parent->name;
        parent_method_name += "&";
        parent_method_name += params_without_self;
        method.name_without_class_prefix = std::move(stripped_name);
        method.name_without_class_prefix += '(' + ((params_without_self != ")") ? params_without_self.substr(2) : ")");
        auto index = parent_vtable->method_indices.find(parent_method_name);
        // If this method is not in the parent vtable,
        //  then it's a method that got introduced in
        //  this class, and will need to have an extra
        //  slot in the vtable. Otherwise, this is a
        //  method that is overriding the parent method,
        //  and will need to overwrite the parent slot
        //  in the vtable.
        if (index == parent_vtable->method_indices.end())
            methods.push_back(method);
        else
            methods[index->second - VTable::RESERVED_FIELDS_COUNT] = method;
    }

    // The indices here are the indices in the vtable, which include the class name pointer and the parent pointer.
    for (auto& [name, index] : parent_vtable->method_indices)
    {
        if (!methods[index - VTable::RESERVED_FIELDS_COUNT].name.empty()) {
            // This method is overwritten by the current class implementation
            continue;
        }

        // Only set the methods that aren't overridden by the current class. Substitute the parent methods.
        auto info = compiler->get_name_resolver().get_function_no_overloading(name);
        auto func = compiler->get_module()->getFunction(name);
        auto type = info.type;
        auto owner = info.owner_class;
        auto self_param_position = name.find(owner->name + "&");  // This must not be npos
        auto stripped_name = name.substr(owner->name.size() + 1, self_param_position - (owner->name.size() + 1 + 1));
        auto params_without_self = name.substr(self_param_position + owner->name.size() + 1);
        std::string name_without_class_prefix = std::move(stripped_name);
        name_without_class_prefix += '(' + ((params_without_self != ")") ? params_without_self.substr(2) : ")");
        methods[index - VTable::RESERVED_FIELDS_COUNT] = { name, func, type, std::move(name_without_class_prefix) };
    }

    return methods;
}

std::vector<TypeAliasNode*>& ClassResolver::get_class_aliases(const std::string& class_name) {
    return class_aliases[class_name];
}

llvm::Value* VTable::get_method(const std::string &name, llvm::Type* type, llvm::Value* instance)
{
    auto it = method_indices.find(name);
    if (it == method_indices.end())
        owner->compiler->report_error("The class " + owner->name + " doesn't contain a method with the name " + name);

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

bool VTable::has_method(const std::string& name) const
{
    // Constructor methods are not included in the map
    if (name == "constructor") return true;

    auto key = name + '(';
    auto begin = method_names_without_class_prefix.lower_bound(key);
    key.back()++;
    auto end = method_names_without_class_prefix.lower_bound(key);
    return begin != end;
}

}