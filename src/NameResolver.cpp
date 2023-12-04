#include <NameResolver.hpp>
#include <ModuleCompiler.hpp>
#include <llvm/IR/Verifier.h>

namespace dua
{

struct FieldInfo
{
    size_t index;
    ClassField& info;
};

NameResolver::NameResolver(ModuleCompiler *compiler) : compiler(compiler) {}

void NameResolver::add_fields_constructor_args(std::string class_name, std::vector<FieldConstructorArgs> args)
{
    fields_args[std::move(class_name)] = std::move(args);
}

std::vector<FieldConstructorArgs> &NameResolver::get_fields_args(const std::string &class_name) {
    auto it = fields_args.find(class_name);
    if (it == fields_args.end())
        report_internal_error("Class " + class_name + " is not defined yet");
    return it->second;
}

void NameResolver::register_function(std::string name, FunctionInfo info)
{
    if (compiler->current_function != nullptr)
        report_internal_error("Nested functions are not allowed");

    // Registering the function in the module so that all
    //  functions are visible during AST evaluation.
    llvm::Type* ret = info.type.return_type->llvm_type();
    std::vector<llvm::Type*> parameter_types;
    for (auto& type: info.type.param_types)
        parameter_types.push_back(type->llvm_type());
    llvm::FunctionType* type = llvm::FunctionType::get(ret, parameter_types, info.type.is_var_arg);
    llvm::Function* function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, compiler->module);
    llvm::verifyFunction(*function);

    auto it = functions.find(name);
    if (it != functions.end()) {
        // Verify that the signatures are the same
        if (it->second.type != info.type) {
            report_error("Redefinition of the function " + name + " with a different signature");
        }
    } else {
        functions[std::move(name)] = std::move(info);
    }
}

FunctionInfo &NameResolver::get_function(const std::string &name)
{
    auto it = functions.find(name);

    if (it != functions.end())
        return it->second;

    size_t dot = 0;
    while (dot < name.size() && name[dot] != '.') dot++;

    if (dot == name.size()) {
        report_error("The function " + name + " is not declared/defined");
    } else {
        // A class method
        auto class_name = name.substr(0, dot);
        auto method_name = name.substr(dot + 1);
        report_error("The class " + class_name + " is not defined. Can't resolve " + class_name + "::" + method_name);
    }

    // Unreachable
    return it->second;
}

bool NameResolver::has_function(const std::string &name) const
{
    return functions.find(name) != functions.end();
}

void NameResolver::cast_function_args(std::vector<llvm::Value *> &args, const FunctionType &type)
{
    for (size_t i = args.size() - 1; i != (size_t)-1; i--) {
        if (i < type.param_types.size()) {
            // Only try to cast non-var-arg parameters
            auto param_type = type.param_types[i]->llvm_type();
            args[i] = compiler->cast_value(args[i], param_type);
        } else if (args[i]->getType() == builder().getFloatTy()) {
            // Variadic functions promote floats to doubles
            args[i] = compiler->cast_value(args[i], builder().getDoubleTy());
        }
    }
}

llvm::CallInst *NameResolver::call_function(const std::string &name, std::vector<llvm::Value *> args)
{
    auto function = compiler->module.getFunction(name);
    if (function == nullptr)
        report_error("The function " + name + " is not defined");
    auto& info = get_function(name);
    cast_function_args(args, info.type);
    return builder().CreateCall(function, args);
}

llvm::CallInst* NameResolver::call_function(llvm::Value *ptr, const FunctionType &type, std::vector<llvm::Value *> args)
{
    cast_function_args(args, type);
    return builder().CreateCall(type.llvm_type(), ptr, args);
}

void NameResolver::call_method_if_exists(const Variable &variable, const std::string &name, std::vector<llvm::Value*> args)
{
    if (auto class_type = dynamic_cast<ClassType *>(variable.type); class_type != nullptr) {
        std::string full_name = class_type->name + "." + name;
        if (functions.find(full_name) != functions.end()) {
            args.insert(args.begin(), variable.ptr);
            call_function(full_name, std::move(args));
        }
    }
}

void NameResolver::call_constructor(const Variable &variable, std::vector<llvm::Value *> args)
{
    if (!variable.ptr->getType()->isPointerTy())
        report_internal_error("Trying to initialize a non-lvalue item");

    auto class_type = dynamic_cast<ClassType*>(variable.type);

    if (class_type == nullptr)
    {
        // Initializing a primitive type.
        if (args.empty())
            return;

        else if (args.size() != 1)
            report_error("Cannot initialize a primitive value with more than one value");

        auto value = compiler->cast_value(args[0], variable.type->llvm_type());
        builder().CreateStore(value, variable.ptr);

        return;
    }

    // Initializing an object

    std::string name = class_type->name + ".constructor";
    if (!has_function(name)) {
        // Constructor is not defined. Noting to do here
        return;
    }

    // Fields constructors are called in the function definition
    //  node. This is to ensure that the parameters are defined
    //  and are visible to these constructors, so that they can
    //  be passed as well. This is to have the constructor calls
    //  happen inside the constructor, not in every caller site.
    //  These calls will happen at the top of the constructor,
    //  thus, initializing the fields first.

    args.insert(args.begin(), variable.ptr);
    call_function(name, args);
}

void NameResolver::call_destructor(const Variable &variable)
{
    auto class_type = dynamic_cast<ClassType*>(variable.type);
    if (class_type == nullptr)
        return;

    std::string name = class_type->name + ".destructor";
    if (!has_function(name))
        return;

    // Call the destructors of fields first
    // Fields are destructed in the reverse order of definition.
    std::for_each(class_type->fields().rbegin(), class_type->fields().rend(), [&](auto& field) {
        // TODO don't search for the field twice, once for the type and once for the ptr
        auto type = class_type->get_field(field.name).type;
        auto ptr = class_type->get_field(variable.ptr, field.name);
        call_destructor({ ptr, type });
    });

    call_function(name, { variable.ptr });
}

void NameResolver::destruct_all_variables(const Scope<Variable> &scope)
{
    // TODO enforce an order on the destruction
    for (auto& variable : scope.map)
        call_destructor(variable.second);
}

void NameResolver::push_scope()
{
    symbol_table.push_scope();
}

Scope<Variable> NameResolver::pop_scope()
{
    return symbol_table.pop_scope();
}

NameResolver::~NameResolver()
{
    for (auto& cls : classes)
        delete cls.second;
}

llvm::IRBuilder<>& NameResolver::builder() const
{
    return compiler->builder;
}

ClassType *NameResolver::get_class(const std::string &name) {
     return classes[name];
}

}