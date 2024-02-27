#include <resolution/FunctionNameResolver.hpp>
#include <ModuleCompiler.hpp>
#include <llvm/IR/Verifier.h>
#include "types/FloatTypes.hpp"
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"
#include "types/VoidType.hpp"
#include "types/IntegerTypes.hpp"
#include "types/ArrayType.hpp"

namespace dua
{

std::vector<const Type*> get_types(const std::vector<Value>& args)
{
    std::vector<const Type*> types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        types[i] = args[i].type;
    return types;
}

void FunctionNameResolver::report_function_not_defined(const std::string& name)
{
    compiler->report_error("The function " + name + " is not declared/defined");
}

FunctionNameResolver::FunctionNameResolver(ModuleCompiler *compiler) : compiler(compiler) {}

void FunctionNameResolver::register_function(std::string name, FunctionInfo info, bool nomangle)
{
    if (compiler->current_function != nullptr)
        compiler->report_internal_error("Nested functions are not allowed");

    // Registering the function in the module so that all
    //  functions are visible during AST evaluation.

    // Concrete types must be captured when registering
    info.type = info.type->with_concrete_types();

    if (!nomangle)
        name = get_function_full_name(name, info.type->param_types);

    if (compiler->name_resolver.has_class(name))
        compiler->report_error("There is already a class with the name " + name + ". Can't have a function with the same name");

    auto it = functions.find(name);
    if (it != functions.end()) {
        // Verify that the signatures are the same
        if (it->second.type != info.type) {
            compiler->report_error("Redefinition of the function " + name + " with a different signature");
        }
    } else {
        llvm::FunctionType* type = info.type->llvm_type();
        llvm::Function* function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, compiler->module);
        llvm::verifyFunction(*function);
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

std::string FunctionNameResolver::get_function_full_name(std::string name, bool try_as_method, bool panic_on_error)
{
    if (try_as_method && compiler->current_class != nullptr) {
        auto class_name = compiler->current_class->getName().str();
        auto result = get_function_full_name(class_name + "." + name, false, false);
        if (!result.empty()) return result;
    }
    name += '(';
    auto begin = functions.lower_bound(name);
    name.back()++;
    auto end = functions.lower_bound(name);
    name.pop_back();

    auto non_mangled = functions.find(name);

    if (begin == end) {
        if (non_mangled == functions.end()) {
            if (panic_on_error)
                report_function_not_defined(name);
            return "";
        }
        return name;
    }

    // There exists more than one overload
    if (non_mangled != functions.end() || --end != begin) {
        if (panic_on_error)
            compiler->report_error("Can't infer the overloaded function '" + name +
               "' just from the name. Specifying the function type might help resolve this conflict");
        return "";
    }

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
    if (compiler->name_resolver.has_class(name))
        return false;

    if (try_as_method && compiler->current_class) {
        auto class_name = compiler->current_class->getName().str();
        if (has_function(class_name + "." + name, false))
            return true;
    }

    // Non-mangled functions
    if (functions.find(name) != functions.end())
        return true;

    auto key = name + "(";
    auto begin = functions.lower_bound(key);
    key.back()++;
    auto end = functions.lower_bound(key);
    return begin != end;
}

void FunctionNameResolver::cast_function_args(std::vector<Value> &args, const FunctionType* type) const
{
    for (size_t i = args.size() - 1; i != (size_t)-1; i--)
    {
        auto arg_type = args[i].type;
        auto arg_ref = arg_type->as<ReferenceType>();

        if (i < type->param_types.size())
        {
            // Only try to cast non-var-arg parameters

            auto param_type = type->param_types[i];
            auto param_ref = param_type->as<ReferenceType>();

            if (param_ref != nullptr)
            {
                if (arg_ref != nullptr) {
                    if (!arg_ref->is_allocated()) {
                        // If the arg is an unallocated reference, turn it into an
                        //  allocated reference, since it's leaving the local scope
                        assert(args[i].memory_location != nullptr);
                        args[i].set(args[i].memory_location);
                        args[i].type = arg_ref->get_allocated();
                    }
                } else {
                    // Turn the argument into reference by passing its memory_location instead
                    if (args[i].memory_location == nullptr)
                        compiler->report_error("Can't pass a non-lvalue expression (with " + arg_type->to_string() + " type) as a reference");
                    args[i].set(args[i].memory_location);
                    args[i].type = compiler->create_type<ReferenceType>(arg_type, true);
                    args[i].memory_location = nullptr;
                }

            }
            else
            {
                // Allocated references are already loaded.
                // If the parameter is not a reference type, and the
                //  argument is a reference, set the pointer as the
                //  memory_location
                if (arg_ref && arg_ref->is_allocated()) {
                    args[i].memory_location = args[i].get();
                    args[i].set(nullptr);
                    args[i].type = arg_ref->get_element_type();
                }
            }

            args[i] = compiler->typing_system.cast_value(args[i], param_type);
        }
        else
        {
            if (arg_ref && arg_ref->is_allocated()) {
                // Allocated references are already loaded.
                // References are passed as pointers, but there are no references in
                //  variadic arguments, thus, load the value and pass it by value.
                args[i].memory_location = args[i].get();
                args[i].set(nullptr);
                args[i].type = arg_ref->get_contained_type();
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

Value FunctionNameResolver::call_function(const std::string &name, std::vector<Value> args, Value* out_result)
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
        compiler->report_error("The function " + name + " is not defined");

    auto& info = get_function_no_overloading(full_name);

    auto result_func = compiler->create_value(function, info.type);

    if (out_result != nullptr) {
        *out_result = result_func;
    }

    return call_function(result_func, std::move(args));
}

Value FunctionNameResolver::call_function(const Value& func, std::vector<Value> args)
{
    auto type = func.type->as<FunctionType>();
    if (type == nullptr)
        compiler->report_internal_error("Calling a non-function type (a " + func.type->to_string() + ")");

    cast_function_args(args, type);

    // As of now, variadic functions take only primitive
    //  values. There is no need to call the copy constructor
    //  for variadic arguments
    for (size_t i = 0; i < type->param_types.size(); i++)
    {
        // After creating the argument and calling the copy constructor, pass
        //  it to the function.
        // At the function definition side, all arguments will be teleported,
        //  because their copy constructor is already called (if needed), and
        //  it shouldn't get called again.
        args[i] = compiler->get_bound_value(args[i], type->param_types[i]);
    }

    std::vector<llvm::Value*> llvm_args(args.size());
    for (size_t i = 0; i < args.size(); i++)
        llvm_args[i] = args[i].get();

    auto return_value = builder().CreateCall(type->llvm_type(), func.get(), std::move(llvm_args));

    auto result = compiler->create_value(return_value, type->return_type);

    // The result of a function call is always
    //  teleporting. If a copy constructor is
    //  needed, it's called at the function
    //  before returning the result.
    result.is_teleporting = true;

    if (type->return_type->as<VoidType>() == nullptr) {
        // This relies on result.is_teleporting being true
        auto ptr = compiler->create_local_variable("", type->return_type, &result, {});
        result.set(nullptr);
        result.memory_location = ptr;
        result.id = compiler->get_temp_expr_map_unused_id();
        compiler->insert_temp_expr(result);
    }

    return result;
}

void FunctionNameResolver::call_constructor(const Value &value, std::vector<Value> args)
{
    if (!value.get()->getType()->isPointerTy())
        compiler->report_internal_error("Trying to initialize a non-lvalue item");

    auto class_type = value.type->is<ClassType>();

    if (class_type == nullptr)
    {
        // Initializing a primitive or a reference type.

        if (auto ref = value.type->is<ReferenceType>(); ref != nullptr) {
            // Usually, a reference is just put into the symbol table directly as an alias
            //  for the referenced expression, but sometimes (as in a reference field), you
            //  can't perform this optimization. In this case, you have to store its address
            // TODO report the field name as well
            if (args.empty())
                compiler->report_error("Reference fields must be initialized");
            if (args.size() > 1)
                compiler->report_error("Reference fields expects exactly one referenced argument");
            if (args.front().is_teleporting)
                compiler->report_error("Reference fields can't be bound to a temporary expression, which will be stale by the next statement");
            if (args.front().memory_location == nullptr)
                compiler->report_error("Reference fields can't be assigned a non-lvalue argument");
        } else {
            if (args.empty())
                args.push_back(compiler->create_value(value.type->default_value().get(), value.type));
            else if (args.size() != 1)
                compiler->report_error("Cannot initialize a primitive type with more than one initializer value");
        }

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

    auto instance = compiler->create_value(value.get(), compiler->create_type<ReferenceType>(value.type, true));
    args.insert(args.begin(), instance);

    std::vector<const Type*> arg_types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        arg_types[i] = args[i].type;

    auto name = compiler->name_resolver.get_winning_function(class_type->name + ".constructor", arg_types, false);

    if (name.empty() && args.size() == 2) {
        // Try again as a copy constructor
        name = compiler->name_resolver.get_winning_function(class_type->name + ".=constructor", arg_types, false);
    }

    if (name.empty())
    {
        if (args.size() == 1) {
            // The first argument is the self argument
            compiler->report_error("There is no default constructor defined for the class " + class_type->name);
        }

        auto message = "Neither a constructor nor a copy constructor for the class type "
                + class_type->name + " are applicable with the provided arguments:";

        for (auto& type : arg_types)
            message += "\n" + type->to_string();

        compiler->report_internal_error(message);
    }

    call_function(name, std::move(args));
}

void FunctionNameResolver::copy_construct(const Value &instance, const Value& arg)
{
    // Either calls the copy constructor, or performs a bitwise copy

    if (!instance.get()->getType()->isPointerTy())
        compiler->report_internal_error("Trying to initialize a non-lvalue item");

    auto class_type = instance.type->is<ClassType>();

    // If the instance is teleporting (being moved from one scope to another without
    //  getting destructed), its copy constructor shouldn't be called as well.
    if (class_type != nullptr && !arg.is_teleporting)
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
            auto ref = compiler->create_value(instance.get(), compiler->create_type<ReferenceType>(instance.type, true));
            call_function(name, { ref, arg });
            return;
        }

        // There is no copy constructor defined for
        //  this class. Copy construct every field

        if (!compiler->typing_system.is_castable(arg.type, class_type))
            report_error("Can't copy an object of type " + arg.type->to_string() + " to an object of type "
                + class_type->to_string() + " without defining a copy constructor between the two types");

        // Regardless of where the object to copy is coming
        //  from, the vtable is set to the vtable of the class
        size_t n = class_type->fields().size();
        auto source_vtable = class_type->get_field(instance, 0);
        auto target_vtable = compiler->create_value(
            compiler->name_resolver.get_vtable_instance(class_type->name)->instance,
            compiler->name_resolver.get_vtable_type(class_type->name)
        );
        copy_construct(source_vtable, target_vtable);

        // Ignore the vtable field
        auto other = compiler->create_value(arg.memory_location, arg.type);
        for (size_t i = 1; i < n; i++) {
            auto source = class_type->get_field(instance, i);
            auto target = class_type->get_field(other, i);
            target.memory_location = target.get();
            target.set(nullptr);
            copy_construct(source, target);
        }

        return;
    }

    // Initializing a primitive type

    auto casted = compiler->typing_system.cast_value(arg, instance.type);
    builder().CreateStore(casted.get(), instance.get());
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
        compiler->report_internal_error("A destructor is not defined for the class " + class_type->name);

    auto instance = compiler->create_value(value.get(), compiler->create_type<ReferenceType>(value.type, true));
    auto full_name = get_function_full_name(name, std::vector<const Type*>{ instance.type });
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
    if (name != "main") key += '(';
    auto begin = functions.lower_bound(key);
    key.back()++;
    auto end = functions.lower_bound(key);

    auto non_mangled = functions.find(name);

    if (begin == end) {

        if (non_mangled != functions.end()) {
            // This is a non-mangled function name
            return name;
        }

        if (panic_on_not_found)
            compiler->report_error("Function " + name + " is not defined");

        else return "";
    }

    std::map<int, std::vector<std::pair<std::string, const FunctionType*>>> scores;

    if (non_mangled != functions.end()) {
        auto score = compiler->typing_system.type_list_similarity_score(arg_types, non_mangled->second.type->param_types);
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

        compiler->report_error(message);
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

        compiler->report_error(message);
    }

    return result_list.front().first;
}

std::string FunctionNameResolver::get_function_full_name(std::string name, const std::vector<const Type*> &param_types)
{
    if (name == "main") return name;
    name += '(';
    // is_var_arg is not part of the full name since you can't
    //  have two functions with the same parameters with the
    //  exception that one is a variadic and the other is not
    bool first = true;
    for (auto type : param_types) {
        if (!first) name += ", ";
        name += type->to_string();
        first = false;
    }
    name += ')';
    return name;
}

std::string FunctionNameResolver::get_function_name_with_exact_type(const std::string &name, const FunctionType* type) const
{
    auto mangled_name = get_function_full_name(name, type->param_types);
    auto non_mangled = functions.find(name);
    auto mangled = functions.find(mangled_name);

    if (non_mangled != functions.end() && non_mangled->second.type == type) {
        if (mangled != functions.end())
            compiler->report_error("A resolution conflict between a mangled and a non-mangled functions with name " + name);
        return name;
    } else if (mangled != functions.end()) {
        return mangled_name;
    }

    compiler->report_error("There is no overload for the function '" + name + "' with the type " + type->to_string());

    return "";  // Unreachable
}

Value FunctionNameResolver::call_operator(const std::string& position_name, const Value& lhs, const Value& rhs, const std::string& name)
{
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
    name += ".";
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
        compiler->report_internal_error("Function " + name + " not defined");
    return it->second.is_templated;
}

bool FunctionNameResolver::is_function_a_constructor(const std::string &name)
{
    for (auto& ctor : { ".constructor(", ".=constructor(" }) {
        auto pos = name.find(ctor);
        if (pos != std::string::npos) return true;
    }
    return false;
}

std::string FunctionNameResolver::get_winning_method(const ClassType *owner, const std::string &name,
                                                     const std::vector<const Type *> &arg_types,
                                                     bool panic_on_not_found, bool panic_on_ambiguity) const
{
    if (name == "constructor")
    {
        // Constructors are not stored in the map
        return get_winning_function(owner->name + ".constructor", arg_types, panic_on_not_found, panic_on_ambiguity);
    }

    auto key = name;
    auto vtable = compiler->name_resolver.get_vtable_instance(owner->name);
    auto begin = vtable->method_names_without_class_prefix.lower_bound(key);
    key.back()++;
    auto end = vtable->method_names_without_class_prefix.lower_bound(key);

    if (begin == end)
    {
        if (panic_on_not_found)
            compiler->report_error("Function " + name + " is not defined");

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

        compiler->report_error(message);
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

        compiler->report_error(message);
    }

    return result_list.front().first;
}

void FunctionNameResolver::construct_array(const Value &ptr, size_t element_size, const Value& count, std::vector<Value> args)
{
    // Call the constructor for each element in a loop

    // This must be a pointer type
    auto ptr_type = ptr.type->as<PointerType>();
    auto element_type = ptr_type->get_element_type();
    auto alloc_type = element_type->llvm_type();

    auto condition_bb = compiler->create_basic_block("array_init_condition");
    auto body_bb = compiler->create_basic_block("array_init_loop");
    auto end_bb = compiler->create_basic_block("array_init_end");

    auto as_i64 = count.cast_as(compiler->create_type<I64Type>(), false);
    if (as_i64.is_null())
        compiler->report_error("The type " + count.type->to_string()
                               + " can't be used as the size of an array. (While allocating an array of " + element_type->to_string() + ")");

    llvm::Value* bytes = builder().getInt64(element_size);
    bytes = builder().CreateMul(as_i64.get(), bytes);

    auto counter = compiler->create_local_variable(".array_counter", compiler->create_type<I64Type>(), nullptr);
    builder().CreateBr(condition_bb);

    builder().SetInsertPoint(condition_bb);
    auto counter_val = builder().CreateLoad(builder().getInt64Ty(), counter);
    auto cmp = builder().CreateICmpEQ(counter_val, as_i64.get());
    builder().CreateCondBr(cmp, end_bb, body_bb);

    builder().SetInsertPoint(body_bb);
    auto array = compiler->create_type<ArrayType>(element_type, LONG_LONG_MAX);
    auto instance = builder().CreateGEP(
        array->llvm_type(),
        ptr.get(),
        { builder().getInt32(0), counter_val }
    );
    compiler->get_name_resolver().call_constructor(compiler->create_value(instance, element_type), std::move(args));
    auto inc = builder().CreateAdd(counter_val, builder().getInt64(1));
    builder().CreateStore(inc, counter);
    builder().CreateBr(condition_bb);

    builder().SetInsertPoint(end_bb);
}

void FunctionNameResolver::destruct_array(const Value &ptr, const Value &count)
{
    auto condition_bb = compiler->create_basic_block("delete_destruct_condition");
    auto body_bb = compiler->create_basic_block("delete_destruct_loop");
    auto destruct_end_bb = compiler->create_basic_block("delete_destruct_end");

    auto counter = compiler->create_local_variable(".delete_counter", compiler->create_type<I64Type>(), nullptr);
    builder().CreateBr(condition_bb);

    builder().SetInsertPoint(condition_bb);
    auto counter_val = builder().CreateLoad(builder().getInt64Ty(), counter);
    auto cmp = builder().CreateICmpEQ(counter_val, count.get());
    builder().CreateCondBr(cmp, destruct_end_bb, body_bb);

    builder().SetInsertPoint(body_bb);
    auto element_type = ptr.type->as<PointerType>()->get_element_type();
    auto array = compiler->create_type<ArrayType>(element_type, LONG_LONG_MAX);
    auto instance = builder().CreateGEP(
        array->llvm_type(),
        ptr.get(),
        { builder().getInt32(0), counter_val }
    );
    compiler->get_name_resolver().call_destructor(compiler->create_value(instance, element_type));
    auto inc = builder().CreateAdd(counter_val, builder().getInt64(1));
    builder().CreateStore(inc, counter);
    builder().CreateBr(condition_bb);

    builder().SetInsertPoint(destruct_end_bb);
}

}