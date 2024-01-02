#include <resolution/FunctionNameResolver.hpp>
#include <ModuleCompiler.hpp>
#include <llvm/IR/Verifier.h>
#include "types/FloatTypes.hpp"
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"

namespace dua
{

std::vector<const Type*> get_types(const std::vector<Value>& args)
{
    std::vector<const Type*> types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        types[i] = args[i].type;
    return types;
}

void report_function_not_defined(const std::string& name)
{
    report_error("The function " + name + " is not declared/defined");
}

FunctionNameResolver::FunctionNameResolver(ModuleCompiler *compiler) : compiler(compiler) {}

void FunctionNameResolver::register_function(std::string name, FunctionInfo info, bool nomangle)
{
    if (compiler->current_function != nullptr)
        report_internal_error("Nested functions are not allowed");

    // Registering the function in the module so that all
    //  functions are visible during AST evaluation.

    // Concrete types must be captured when registering
    info.type = info.type->with_concrete_types();

    if (!nomangle)
        name = get_function_full_name(name, info.type->param_types);

    llvm::FunctionType* type = info.type->llvm_type();
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

FunctionInfo& FunctionNameResolver::get_function_no_overloading(const std::string &name)
{
    auto it = functions.find(name);
    if (it != functions.end())
        return it->second;
    report_function_not_defined(name);

    // Unreachable
    return it->second;
}

std::string FunctionNameResolver::get_function_full_name(std::string name)
{
    name += '.';
    auto begin = functions.lower_bound(name);
    name.back()++;
    auto end = functions.lower_bound(name);
    name.pop_back();

    auto non_mangled = functions.find(name);

    if (begin == end) {
        if (non_mangled == functions.end())
            report_function_not_defined(name);
        return name;
    }

    // There exists more than one overload
    if (non_mangled != functions.end() || --end != begin)
        report_error("Can't infer the overloaded function '" + name +
        "' just from the name. Specifying the function type might help resolve this conflict");

    return begin->first;
}

FunctionInfo& FunctionNameResolver::get_function(const std::string &name, const std::vector<const Type*> &param_types)
{
    auto full_name = get_winning_function(name, param_types);
    auto it = functions.find(full_name);
    if (it != functions.end())
        return it->second;
    report_function_not_defined(name);

    // Unreachable
    return it->second;
}

FunctionInfo &FunctionNameResolver::get_function(const std::string &name, const std::vector<Value> &args) {
    return get_function(name, get_types(args));
}

bool FunctionNameResolver::has_function(const std::string &name, bool try_as_method) const
{
    if (try_as_method && compiler->current_class) {
        auto class_name = compiler->current_class->getName().str();
        if (has_function(class_name + "." + name, false))
            return true;
    }

    // Non-mangled functions
    if (functions.find(name) != functions.end())
        return true;

    // Function names given by the user can't contain the '.' character.
    //  This character is added by the compiler to give different names
    //  based on the types of the parameters for functions defined with
    //  the same name. It's sufficient to search for this prefix only
    //  to determine the existence of a function, with no regard to the
    //  parameter types. Note that functions with no parameters are a
    //  special case, but still has a '.' appended to them.
    auto key = name + ".";
    auto begin = functions.lower_bound(key);
    key.back()++;
    auto end = functions.lower_bound(key);
    return begin != end;
}

void FunctionNameResolver::cast_function_args(std::vector<Value> &args, const FunctionType* type) const
{
    for (size_t i = args.size() - 1; i != (size_t)-1; i--) {
        if (i < type->param_types.size())
        {
            // Only try to cast non-var-arg parameters
            auto param_type = type->param_types[i];
            if (auto ref = dynamic_cast<const ReferenceType*>(param_type); ref != nullptr) {
                // If is a reference type, replace it with a pointer type.
                param_type = compiler->create_type<PointerType>(ref->get_element_type());
                if (auto arg_ref = args[i].type->as<ReferenceType>(); arg_ref != nullptr)
                    args[i].memory_location = args[i].get();
                if (args[i].memory_location == nullptr)
                    report_error("Can't pass a non-lvalue expression (with " + args[i].type->to_string() + " type) as a reference");

                // Cast the memory location instead of the actual loaded value
                args[i] = compiler->create_value(args[i].memory_location, param_type);
            }
            args[i] = compiler->typing_system.cast_value(args[i], param_type);
        } else {
            if (auto ref = args[i].type->as<ReferenceType>(); ref != nullptr) {
                // No references in variadic arguments
                args[i].memory_location = args[i].get();
                args[i].turn_to_memory_address();
            }
            if (args[i].get()->getType() == builder().getFloatTy()) {
                // Variadic functions promote floats to doubles
                auto double_type = compiler->create_type<F64Type>();
                args[i] = compiler->create_value(
                    compiler->typing_system.cast_value(args[i], double_type).get(),
                    double_type
                );
            }
        }
    }
}

Value FunctionNameResolver::call_function(const std::string &name, std::vector<Value> args)
{
    auto types = get_types(args);

    std::string full_name;

    if (compiler->current_class != nullptr)
    {
        // If there is a function call, this must be from
        //  within a function. If a class is being evaluated
        //  currently, then this is a method, and the "self"
        //  parameter is defined for sure.

        auto class_name = compiler->current_class->getName().str();
        types.insert(types.begin(), compiler->name_resolver.get_class(class_name));
        full_name = get_winning_function(class_name + "." + name, types, false);
        if (full_name.empty()) {
            // Undo
            types.erase(types.begin());
        } else {
            // Add the argument
            auto instance = compiler->name_resolver.symbol_table.get("self");
            instance.memory_location = instance.get();  // The symbol table stores the address, not the value
            args.insert(args.begin(), instance);
        }
    }

    if (full_name.empty())
        full_name = get_winning_function(name, types);

    auto function = compiler->module.getFunction(full_name);
    if (function == nullptr)
        report_error("The function " + name + " is not defined");

    auto& info = get_function_no_overloading(full_name);

    cast_function_args(args, info.type);
    std::vector<llvm::Value*> llvm_args(args.size());
    for (size_t i = 0; i < args.size(); i++)
        llvm_args[i] = args[i].get();

    auto result = builder().CreateCall(function, std::move(llvm_args));
    return compiler->create_value(result, info.type->return_type);
}

Value FunctionNameResolver::call_function(const Value& func, std::vector<Value> args)
{
    auto type = func.type->as<FunctionType>();
    if (type == nullptr)
        report_internal_error("Calling a non-function type");
    cast_function_args(args, type);
    std::vector<llvm::Value*> llvm_args(args.size());
    for (size_t i = 0; i < args.size(); i++)
        llvm_args[i] = args[i].get();
    auto result = builder().CreateCall(type->llvm_type(), func.get(), std::move(llvm_args));
    return compiler->create_value(result, type->return_type);
}

void FunctionNameResolver::call_constructor(const Value &value, std::vector<Value> args)
{
    if (!value.get()->getType()->isPointerTy())
        report_internal_error("Trying to initialize a non-lvalue item");

    auto class_type = value.type->as<ClassType>();

    if (class_type == nullptr)
    {
        // Initializing a primitive type.

        if (args.empty())
            args.push_back(compiler->create_value(value.type->default_value().get(), value.type));
        else if (args.size() != 1)
            report_error("Cannot initialize a primitive type with more than one value");

        auto casted = compiler->typing_system.cast_value(args[0], value.type);
        builder().CreateStore(casted.get(), value.get());

        return;
    }

    // Initializing an object

    // Fields constructors are called in the function definition
    //  node. This is to ensure that the parameters are defined
    //  and are visible to these constructors, so that they can
    //  be passed as well. This is to have the constructor calls
    //  happen inside the constructor, not in every caller site.
    //  These calls will happen at the top of the constructor,
    //  thus, initializing the fields first.

    std::string name = class_type->name + ".constructor";
    if (!has_function(name))
        report_internal_error("A constructor is not defined for class " + class_type->name);

    auto instance = compiler->create_value(value.get(), compiler->create_type<ReferenceType>(value.type));
    args.insert(args.begin(), instance);
    call_function(name, std::move(args));
}

void FunctionNameResolver::call_copy_constructor(const Value &value, const Value& arg)
{
    if (!value.get()->getType()->isPointerTy())
        report_internal_error("Trying to initialize a non-lvalue item");

    auto class_type = value.type->as<ClassType>();

    if (class_type != nullptr)
    {
        // Initializing an object

        // Fields constructors are called in the function definition
        //  node. This is to ensure that the parameters are defined
        //  and are visible to these constructors, so that they can
        //  be passed as well. This is to have the constructor calls
        //  happen inside the constructor, not in every caller site.
        //  These calls will happen at the top of the constructor,
        //  thus, initializing the fields first.

        std::string name = class_type->name + ".=constructor";
        if (has_function(name)) {
            auto instance = compiler->create_value(value.get(), compiler->create_type<ReferenceType>(value.type));
            call_function(name, { instance, arg });
            return;
        }
    }

    // Initializing a primitive type, or an object with no copy constructor.
    auto casted = compiler->typing_system.cast_value(arg, value.type);
    builder().CreateStore(casted.get(), value.get());
}

void FunctionNameResolver::call_destructor(const Value& value)
{
    auto is_ref = value.type->as<ReferenceType>();
    if (is_ref != nullptr)
        return;

    auto class_type = value.type->as<ClassType>();
    if (class_type == nullptr)
        return;

    std::string name = class_type->name + ".destructor";
    if (!has_function(name))
        report_internal_error("A destructor is not defined for the class " + class_type->name);

    // Call the destructors of fields first
    // Fields are destructed in the reverse order of definition.
    std::for_each(class_type->fields().rbegin(), class_type->fields().rend(), [&](auto& field) {
        // TODO don't search for the field twice, once for the type and once for the ptr
        if (field.name == ".vtable_ptr" || field.name.empty()) return;
        auto f = class_type->get_field(value, field.name);
        call_destructor(f);
    });

    auto instance = compiler->create_value(value.get(), compiler->create_type<ReferenceType>(value.type));
    auto full_name = get_function_full_name(name, { instance.type });
    auto vtable_ptr = class_type->get_field(instance, ".vtable_ptr");
    auto vtable_instance = compiler->builder.CreateLoad(compiler->name_resolver.get_vtable_type(class_type->name)->llvm_type(), vtable_ptr.get());
    auto class_vtable = compiler->name_resolver.get_vtable_instance(class_type->name);
    auto destructor_type = compiler->name_resolver.get_function_no_overloading(full_name).type;
    auto destructor_ptr = class_vtable->get_method(full_name, destructor_type->llvm_type()->getPointerTo(), vtable_instance);
    auto destructor = compiler->create_value(destructor_ptr, destructor_type);
    call_function(destructor, { instance });
}

llvm::IRBuilder<>& FunctionNameResolver::builder() const
{
    return compiler->builder;
}

std::string FunctionNameResolver::get_winning_function(const std::string &name, const std::vector<const Type*> &arg_types, bool panic_on_not_found, bool panic_on_ambiguity) const
{
    auto key = name;
    if (name != "main") key += '.';
    auto begin = functions.lower_bound(key);
    key.back()++;
    auto end = functions.lower_bound(key);

    auto non_mangled = functions.find(name);

    if (begin == end) {

        if (non_mangled != functions.end()) {
            // This is a non-mangled function name, which doesn't have
            //  a '.' followed by the parameter types after its name
            return name;
        }

        if (panic_on_not_found)
            report_error("Function " + name + " is undefined");

        else return "";
    }

    std::map<int, std::vector<std::pair<std::string, const FunctionType*>>> scores;

    if (non_mangled != functions.end()) {
        auto score = compiler->typing_system.type_list_similarity_score(
                non_mangled->second.type->param_types, arg_types);
        if (score != -1)
            scores[score].emplace_back(name, non_mangled->second.type);
    }

    auto current = begin;
    while (current != end)
    {
        auto& name = current->first;
        auto& info = current->second;
        current++;
        auto score = compiler->typing_system.type_list_similarity_score(
            arg_types,
            info.type->param_types,
            info.type->is_var_arg
        );
        if (score == -1) continue;
        scores[score].emplace_back(name, info.type);
    }

    if (scores.empty())
    {
        if (!panic_on_not_found) return "";

        std::string message = "There are no applicable overloads for the function '" + name + "' with ";
        if (arg_types.empty()) {
            message += "no arguments";
        } else {
            message += "the provided arguments:\n(";
            message += arg_types.front()->to_string();
            for (size_t i = 1; i < arg_types.size(); i++) {
                message += ", " + arg_types[i]->to_string();
            }
            message += ")";
        }

        message += "\nFunction overloads are:\n";

        current = begin;
        while (current != end)
            message += current->second.type->to_string() + '\n', current++;
        message.pop_back();  // The extra '\n'

        report_error(message);
    }

    auto& result_list = scores.begin()->second;

    if (result_list.size() != 1)
    {
        if (!panic_on_ambiguity) return "";

        std::string message = "More than one overload of the function '" + name + "' is applicable to the function call."
                                                                                 " Applicable functions are:\n";
        for (auto& overload : result_list)
            message += overload.second->to_string() + '\n';
        message.pop_back();  // The extra '\n'

        report_error(message);
    }

    return result_list.front().first;
}

std::string FunctionNameResolver::get_function_full_name(std::string name, const std::vector<const Type*> &param_types)
{
    if (name == "main") return name;
    if (param_types.empty()) return name + ".";
    // is_var_arg is not part of the full name since you can't
    //  have two functions with the same parameters with the
    //  exception that one is a variadic and the other is not
    for (auto type : param_types)
        name += '.' + type->to_string();
    return name;
}

std::string FunctionNameResolver::get_function_name_with_exact_type(const std::string &name, const FunctionType* type) const
{
    auto mangled_name = get_function_full_name(name, type->param_types);
    auto non_mangled = functions.find(name);
    auto mangled = functions.find(mangled_name);

    if (non_mangled != functions.end() && non_mangled->second.type == type) {
        if (mangled != functions.end())
            report_error("A resolution conflict between a mangled and a non-mangled functions with name " + name);
        return name;
    } else if (mangled != functions.end()) {
        return mangled_name;
    }

    report_error("There is no overload for the function '" + name + "' with the type " + type->to_string());

    return "";  // Unreachable
}

Value FunctionNameResolver::call_operator(const std::string& position_name, const Value& lhs, const Value& rhs, const std::string& name) {
    auto full_name = get_winning_function(position_name + "." + name, { lhs.type, rhs.type }, false);
    if (full_name.empty())
        return {};
    auto func = compiler->module.getFunction(full_name);
    auto type = functions[full_name].type;
    auto value = compiler->create_value(func, type);
    return call_function(value, {lhs, rhs});
}

Value FunctionNameResolver::call_infix_operator(const Value &lhs, const Value &rhs, const std::string& name) {
    return call_operator("infix", lhs, rhs, name);
}

Value FunctionNameResolver::call_postfix_operator(const Value &lhs, const Value &rhs, const std::string& name) {
    return call_operator("postfix", lhs, rhs, name);
}

const Type *FunctionNameResolver::get_operator_return_type(const std::string& position_name, const Type *t1, const Type *t2, const std::string& name)
{
    auto full_name = get_winning_function(position_name + "." + name, { t1, t2 }, false);
    if (full_name.empty())
        return nullptr;
    return functions[full_name].type->return_type;
}

const Type *FunctionNameResolver::get_infix_operator_return_type(const Type *t1, const Type *t2, const std::string& name) {
    return get_operator_return_type("infix", t1, t2, name);
}

const Type *FunctionNameResolver::get_postfix_operator_return_type(const Type *t1, const Type *t2, const std::string& name) {
    return get_operator_return_type("postfix", t1, t2, name);
}

std::vector<NamedFunctionValue> FunctionNameResolver::get_class_methods(std::string name, bool for_a_vtable)
{
    auto begin = functions.lower_bound(name);
    name.back()++;
    auto end = functions.lower_bound(name);
    std::vector<NamedFunctionValue> result;
    while (begin != end)
    {
        if (!for_a_vtable || (!is_function_templated(begin->first) && !is_function_a_constructor(begin->first))) {
            auto func = compiler->module.getFunction(begin->first);
            result.push_back({begin->first, func, begin->second.type});
        }
        begin++;
    }
    return result;
}

bool FunctionNameResolver::is_function_templated(const std::string &name) {
    auto it = functions.find(name);
    if (it == functions.end())
        report_internal_error("Function " + name + " not defined");
    return it->second.is_templated;
}

bool FunctionNameResolver::is_function_a_constructor(const std::string &name)
{
    for (auto& ctor : { ".constructor.", ".=constructor." }) {
        auto pos = name.find(ctor);
        if (pos != std::string::npos) return true;
    }
    return false;
}

std::string FunctionNameResolver::get_winning_method(const ClassType *owner, const std::string &name,
                                                     const std::vector<const Type *> &arg_types,
                                                     bool panic_on_not_found, bool panic_on_ambiguity) const
{
    auto key = name;
    auto vtable = compiler->name_resolver.get_vtable_instance(owner->name);
    auto begin = vtable->method_names_without_class_prefix.lower_bound(key);
    key.back()++;
    auto end = vtable->method_names_without_class_prefix.lower_bound(key);

    if (begin == end)
    {
        if (panic_on_not_found)
            report_error("Function " + name + " is undefined");

        else return "";
    }

    std::map<int, std::vector<std::pair<std::string, const FunctionType*>>> scores;

    auto current = begin;
    while (current != end)
    {
        auto& name = current->second;
        auto& info = compiler->name_resolver.get_function_no_overloading(name);
        current++;
        auto score = compiler->typing_system.type_list_similarity_score(
                arg_types,
                info.type->param_types,
                info.type->is_var_arg
        );
        if (score == -1) continue;
        scores[score].emplace_back(name, info.type);
    }

    if (scores.empty())
    {
        if (!panic_on_not_found) return "";

        std::string message = "There are no applicable overloads for the function '" + name + "' with ";
        if (arg_types.empty()) {
            message += "no arguments";
        } else {
            message += "the provided arguments:\n(";
            message += arg_types.front()->to_string();
            for (size_t i = 1; i < arg_types.size(); i++) {
                message += ", " + arg_types[i]->to_string();
            }
            message += ")";
        }

        message += "\nFunction overloads are:\n";

        current = begin;
        while (current != end) {
            auto& info = compiler->name_resolver.get_function_no_overloading(current->first);
            message += info.type->to_string() + '\n', current++;
        }
        message.pop_back();  // The extra '\n'

        report_error(message);
    }

    auto& result_list = scores.begin()->second;

    if (result_list.size() != 1)
    {
        if (!panic_on_ambiguity) return "";

        std::string message = "More than one overload of the function '" + name + "' is applicable to the function call."
                                                                                  " Applicable functions are:\n";
        for (auto& overload : result_list)
            message += overload.second->to_string() + '\n';
        message.pop_back();  // The extra '\n'

        report_error(message);
    }

    return result_list.front().first;
}

}