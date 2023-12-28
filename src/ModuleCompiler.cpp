#include <ModuleCompiler.hpp>
#include <parsing/ParserFacade.hpp>
#include <AST/TranslationUnitNode.hpp>
#include <llvm/Support/Host.h>
#include "AST/function/FunctionDefinitionNode.hpp"
#include "AST/class/ClassDefinitionNode.hpp"
#include "types/ReferenceType.hpp"
#include "AST/variable/ClassFieldDefinitionNode.hpp"

namespace dua
{

ModuleCompiler::ModuleCompiler(const std::string &module_name, const std::string &code) :
    context(),
    module(module_name, context),
    builder(context),
    temp_builder(context),
    name_resolver(this),
    typing_system(this)
{
    module.setTargetTriple(llvm::sys::getDefaultTargetTriple());

    ParserFacade parser(*this);

    function_scope_count.push_back(0);

    // Parse
    TranslationUnitNode* ast = parser.parse(code);

    create_dua_init_function();

    // Generate LLVM IR
    ast->eval();

    complete_dua_init_function();

    llvm::raw_string_ostream stream(result);
    module.print(stream, nullptr);
    result = stream.str();
}

void ModuleCompiler::create_dua_init_function()
{
    auto info = FunctionInfo {
        create_type<FunctionType>(create_type<VoidType>()),
        {}
    };

    name_resolver.register_function(".dua.init", std::move(info));

    auto function = module.getFunction(".dua.init.");
    llvm::BasicBlock::Create(context, "entry", function);
}

void ModuleCompiler::complete_dua_init_function()
{
    auto dua_init = module.getFunction(".dua.init.");
    auto& init_ip = dua_init->getEntryBlock();
    builder.SetInsertPoint(&init_ip);
    for (auto node : deferred_nodes)
        node->eval();

    if (dua_init->begin()->begin() == dua_init->begin()->end()) {
        // The function is empty. Delete it for clarity.
        dua_init->removeFromParent();
    } else {
        // Return
        builder.CreateRetVoid();

        // Call from main.
        auto& main_ip = module.getFunction("main")->getEntryBlock().front();
        builder.SetInsertPoint(&main_ip);
        builder.CreateCall(dua_init);
    }
}

ModuleCompiler::~ModuleCompiler()
{
    for (auto node : nodes)
        delete node;
}

void ModuleCompiler::push_scope() {
    name_resolver.push_scope();
    typing_system.push_scope();
    function_scope_count.back()++;
}

Scope<Value> ModuleCompiler::pop_scope() {
    function_scope_count.back()--;
    typing_system.pop_scope();
    return name_resolver.pop_scope();
}

void ModuleCompiler::destruct_last_scope()
{
    if (builder.GetInsertBlock()->getTerminator() != nullptr)
        return;

    name_resolver.destruct_all_variables(name_resolver.symbol_table.scopes.back());
}

void ModuleCompiler::destruct_function_scope()
{
    if (builder.GetInsertBlock()->getTerminator() != nullptr)
        return;

    auto& scopes = name_resolver.symbol_table.scopes;
    auto n = scopes.size();
    for (size_t i = 1; i <= function_scope_count.back(); i++)
        name_resolver.destruct_all_variables(scopes[n - i]);
}

void ModuleCompiler::push_scope_counter() {
    function_scope_count.push_back(0);
}

void ModuleCompiler::pop_scope_counter() {
    function_scope_count.pop_back();
}

std::string ModuleCompiler::get_templated_function_key(std::string name, size_t args_count) {
    // Templated functions are prefixed with a special keyword to avoid
    // ambiguity with non-templated functions (to not have the same prefix)
    return "Templated." + name + "." + std::to_string(args_count);
}

void ModuleCompiler::add_templated_function(FunctionDefinitionNode* node, std::vector<std::string> template_params, FunctionInfo info, const std::string& class_name, bool in_templated_class)
{
    auto name = get_templated_function_key(node->name, template_params.size());
    auto& functions = templated_functions[std::move(name)];
    for (auto& function : functions) {
        if (*function.info.type == *info.type) {
            report_error("Redefinition of the templated function " + node->name + " with the "
                         + std::to_string(template_params.size()) + " template parameters and the signature " +
                         info.type->to_string());
        }
    }
    auto cls = class_name.empty() ? nullptr : llvm::StructType::getTypeByName(context, class_name);
    functions.push_back({node, std::move(template_params), std::move(info), cls, in_templated_class });
}

Value ModuleCompiler::get_templated_function(const std::string& name, std::vector<const Type *> &template_args, const std::vector<const Type *> &arg_types, bool use_arg_types)
{
    // Templated functions are named as follows:
    //  original_name.template_param_count.param_types_separated_by_dots
    // We need to include the count of the template parameters count to
    //  allow for functions with same signature, but with different number
    //  of template parameters to exist without collision

    auto it = templated_functions.find(get_templated_function_key(name, template_args.size()));

    if (it == templated_functions.end())
        report_error("The templated function " + name + " with "
                     + std::to_string(template_args.size()) + " template parameters is not defined");

    templated_definition_depth++;

    // Finding the best overload
    name_resolver.symbol_table.keep_only_first_n_scopes(templated_definition_depth);
    typing_system.identifier_types.keep_only_first_n_scopes(templated_definition_depth);

    auto& functions = it->second;

    auto& any_overload = functions.front();
    if (any_overload.in_templated_class)
    {
        // If one overload belongs to a class, all other overloads will belong to it too.
        typing_system.push_scope();
        auto class_name = any_overload.owner_class->getName().str();
        auto& bindings = templated_class_bindings[class_name];
        for (size_t i = 0; i < bindings.params.size(); i++)
            typing_system.insert_type(bindings.params[i], bindings.args[i]);
    }

    long long idx = 0;
    if (!use_arg_types) {
        if (functions.size() > 1)
            report_error("Can't infer the correct overload for the templated function " + name + " without the knowledge of the argument types");
    } else {
        idx = get_winner_templated_function(name, functions, template_args, arg_types);
    }

    auto& templated = functions[idx];

    // A scope that will hold the bindings of the template
    //  parameters to the template arguments, and will
    //  shadow the outer scope types that collide if exists.
    typing_system.push_scope();

    for (size_t i = 0; i < template_args.size(); i++)
        typing_system.insert_type(templated.template_params[i], template_args[i]);

    std::string full_name;

    if (use_arg_types)
        full_name = get_templated_function_full_name(name, template_args, templated.info.type->param_types);
    else
        full_name = get_templated_function_full_name(name, template_args);

    if (name_resolver.has_function(full_name))
    {
        auto func = module.getFunction(full_name);
        auto type = name_resolver.get_function_no_overloading(full_name).type;

        typing_system.pop_scope();
        typing_system.identifier_types.restore_prev_state();
        name_resolver.symbol_table.restore_prev_state();
        templated_definition_depth--;

        return create_value(func, type);
    }

    // Function is not defined yet.
    // Instantiate the function with the provided arguments

    // Types shouldn't be cached during the evaluation of templated functions
    auto old_type_cache_config = stop_caching_types;
    stop_caching_types = true;

    // The full name is computed here.
    templated.node->nomangle = true;

    // Temporarily reset the current function to nullptr
    // so that we don't get the nested functions error.
    auto old_func = current_function;
    current_function = nullptr;

    // Temporarily set the current class to the owner
    // class so that method definitions are correct
    auto old_class = current_class;
    // Templated classes will set the current class
    //  before proceeding
    if (!templated.in_templated_class)
        current_class = templated.owner_class;

    name_resolver.register_function(full_name, templated.info, true);

    std::swap(templated.node->name, full_name);
    auto old_template_param_count = templated.node->template_param_count;
    templated.node->template_param_count = -1;
    templated.node->eval();
    templated.node->template_param_count = old_template_param_count;
    std::swap(templated.node->name, full_name);

    current_class = old_class;
    current_function = old_func;

    typing_system.pop_scope();

    typing_system.identifier_types.restore_prev_state();
    name_resolver.symbol_table.restore_prev_state();

    stop_caching_types = old_type_cache_config;
    templated_definition_depth--;

    auto func = module.getFunction(full_name);
    auto type = name_resolver.get_function_no_overloading(full_name).type;

    return create_value(func, type);
}

Value ModuleCompiler::get_templated_function(const std::string& name, std::vector<const Type *> &template_args) {
    return get_templated_function(std::move(name), template_args, std::vector<const Type*>{}, false);
}

std::string ModuleCompiler::get_templated_function_full_name(std::string name, const std::vector<const Type *> &template_args)
{
    // The same way the param types are added to the name, the template args are added to the name.
    name = get_templated_function_key(std::move(name), template_args.size());
    if (!template_args.empty())
        name = name_resolver.get_function_full_name(name, template_args);
    return name;
}

std::string ModuleCompiler::get_templated_function_full_name(std::string name, const std::vector<const Type *> &template_args, const std::vector<const Type *> &param_types) {
    name = get_templated_function_full_name(std::move(name), template_args);
    return name_resolver.get_function_full_name(name, param_types);
}

long long ModuleCompiler::get_winner_templated_function(const std::string& name, const std::vector<TemplatedFunctionNode> &functions, const std::vector<const Type*>& template_args, const std::vector<const Type *> &arg_types, bool panic_on_not_found)
{
    std::map<int, std::vector<long long>> scores;

    for (size_t i = 0; i < functions.size(); i++)
    {
        typing_system.push_scope();

        for (size_t j = 0; j < template_args.size(); j++)
            typing_system.insert_type(functions[i].template_params[j], template_args[j]);

        auto score = typing_system.type_list_similarity_score(
                functions[i].info.type->param_types,
                arg_types,
                functions[i].info.type->is_var_arg
        );

        if (score == -1) continue;

        scores[score].emplace_back(i);

        typing_system.pop_scope();
    }

    if (scores.empty())
    {
        if (!panic_on_not_found) return -1;

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

        for (auto& func : functions)
            message += func.info.type->to_string() + '\n';
        message.pop_back();  // The extra '\n'

        report_error(message);
    }

    auto& result_list = scores.begin()->second;

    if (result_list.size() != 1)
    {
        if (!panic_on_not_found) return -1;

        std::string message = "More than one overload of the function '" + name + "' is applicable to the function call."
                                                                                  " Applicable functions are:\n";
        for (auto idx : result_list)
            message += functions[idx].info.type->to_string() + '\n';
        message.pop_back();  // The extra '\n'

        report_error(message);
    }

    return result_list.front();
}

std::string ModuleCompiler::get_templated_class_key(std::string name, size_t args_count) {
    return name + "." + std::to_string(args_count);
}

std::string ModuleCompiler::get_templated_class_full_name(const std::string& name, const std::vector<const Type*>& template_args) {
    return get_templated_function_full_name(name, template_args);
}

void ModuleCompiler::add_templated_class(ClassDefinitionNode* node, std::vector<std::string> template_params)
{
    auto name = get_templated_class_key(node->name, template_params.size());
    if (templated_classes.find(name) != templated_classes.end())
        report_error("Redefinition of the templated class " + node->name + " with " + std::to_string(template_params.size()) + " template parameters");
    templated_classes[std::move(name)] = {node, std::move(template_params)};
}

const ClassType* ModuleCompiler::get_templated_class(const std::string &name, const std::vector<const Type*>& template_args)
{
    auto key = get_templated_class_key(name, template_args.size());
    auto it = templated_classes.find(key);
    if (it == templated_classes.end())
        report_error("The templated class " + name + " with " + std::to_string(template_args.size()) + " template parameters is not defined");

    auto& templated = it->second;

    templated_definition_depth++;

    name_resolver.symbol_table.keep_only_first_n_scopes(templated_definition_depth);
    typing_system.identifier_types.keep_only_first_n_scopes(templated_definition_depth);

    // A scope that will hold the bindings of the template
    //  parameters to the template arguments, and will
    //  shadow the outer scope types that collide if exists.
    typing_system.push_scope();

    for (size_t i = 0; i < template_args.size(); i++)
        typing_system.insert_type(templated.template_params[i], template_args[i]);

    auto full_name = get_templated_class_full_name(name, template_args);

    if (!name_resolver.has_class(full_name))
        report_internal_error("The templated class " + full_name + " is not defined");

    auto cls = name_resolver.get_class(full_name);

    typing_system.pop_scope();
    typing_system.identifier_types.restore_prev_state();
    name_resolver.symbol_table.restore_prev_state();
    templated_definition_depth--;

    return cls;
}

void ModuleCompiler::add_templated_class_method_info(const std::string &cls, FunctionDefinitionNode* method, FunctionInfo info, std::vector<std::string> template_params) {
    // TODO compute the key as the hash of the triple { name, template param count, type pointer }
    auto key = cls + "." + method->name;
    if (method->template_param_count != -1)
        key += "." + std::to_string(method->template_param_count);
    key += info.type->as_key();
    templated_class_method_info[key] = { std::move(info), std::move(template_params) };
}

TemplatedClassMethodInfo ModuleCompiler::get_templated_class_method_info(const std::string &cls, const std::string &method, const FunctionType* type, size_t template_param_count) {
    auto key = cls + "." + method;
    if (template_param_count != -1)
        key += "." + std::to_string(template_param_count);
    key += type->as_key();
    auto it = templated_class_method_info.find(key);
    if (it == templated_class_method_info.end())
        report_internal_error("There is no info stored for the method " + method + " of the templated class " + cls);
    return it->second;
}

void ModuleCompiler::register_templated_class(const std::string &name, const std::vector<const Type *> &template_args)
{
    auto key = get_templated_class_key(name, template_args.size());
    auto it = templated_classes.find(key);
    if (it == templated_classes.end())
        report_error("The templated class " + name + " with " + std::to_string(template_args.size()) + " template parameters is not defined");

    auto& templated = it->second;

    auto full_name = get_templated_class_full_name(name, template_args);

    auto cls = create_type<ClassType>(full_name);
    name_resolver.classes[full_name] = cls;
    typing_system.insert_type(full_name, cls);

    templated_class_bindings[full_name] = { templated.template_params, template_args };

    // Registering the methods of the class

    templated_definition_depth++;

    name_resolver.symbol_table.keep_only_first_n_scopes(templated_definition_depth);
    typing_system.identifier_types.keep_only_first_n_scopes(templated_definition_depth);

    // A scope that will hold the bindings of the template
    //  parameters to the template arguments, and will
    //  shadow the outer scope types that collide if exists.
    typing_system.push_scope();

    for (size_t i = 0; i < template_args.size(); i++)
        typing_system.insert_type(templated.template_params[i], template_args[i]);

    auto& methods = templated.node->methods;
    for (auto & method : methods)
    {
        auto method_name = full_name + "." + method->name;

        auto new_param_types = method->function_type->param_types;
        // The first element is a placeholder
        new_param_types[0] = create_type<ReferenceType>(cls);
        auto ret = method->function_type->return_type;
        auto is_var_arg = method->function_type->is_var_arg;

        auto concrete_type = create_type<FunctionType>(ret, new_param_types, is_var_arg);

        auto [info, template_params] = get_templated_class_method_info(key, method->name, method->function_type, method->template_param_count);
        info.type = concrete_type;

        if (method->template_param_count != -1) {
            // Cloning the method node here because we're going to restore its function type later
            auto node = create_node<FunctionDefinitionNode>(method_name, method->body, concrete_type, method->nomangle, method->template_param_count);
            add_templated_function(node, std::move(template_params), std::move(info), full_name, true);
        } else {
            // Setting the full name after the substitution of the concrete class type
            auto full_method_name = name_resolver.get_function_full_name(method_name, concrete_type->param_types);
            name_resolver.register_function(std::move(full_method_name), std::move(info), true);
        }
    }

    typing_system.pop_scope();

    templated_definition_depth--;

    typing_system.identifier_types.restore_prev_state();
    name_resolver.symbol_table.restore_prev_state();
}

const ClassType* ModuleCompiler::define_templated_class(const std::string &name, const std::vector<const Type *> &template_args)
{
    auto key = get_templated_class_key(name, template_args.size());
    auto it = templated_classes.find(key);
    if (it == templated_classes.end())
        report_error("The templated class " + name + " with " + std::to_string(template_args.size()) + " template parameters is not defined");

    auto& templated = it->second;

    templated_definition_depth++;

    name_resolver.symbol_table.keep_only_first_n_scopes(templated_definition_depth);
    typing_system.identifier_types.keep_only_first_n_scopes(templated_definition_depth);

    // A scope that will hold the bindings of the template
    //  parameters to the template arguments, and will
    //  shadow the outer scope types that collide if exists.
    typing_system.push_scope();

    for (size_t i = 0; i < template_args.size(); i++)
        typing_system.insert_type(templated.template_params[i], template_args[i]);

    auto full_name = get_templated_class_full_name(name, template_args);

    // Class is not defined yet.
    // Instantiate the class with the provided arguments

    // Types shouldn't be cached during the evaluation of templated classes
    auto old_type_cache_config = stop_caching_types;
    stop_caching_types = true;

    // Temporarily set the current function to nullptr
    // so that we don't get the nested functions error.
    auto old_func = current_function;
    current_function = nullptr;

    // Temporarily set the current class to nullptr so
    // that we don't get the nested class definition error.
    auto old_class = current_class;
    current_class = nullptr;

    auto cls = name_resolver.get_class(full_name);

    auto& methods = templated.node->methods;
    std::vector<std::string> old_names(methods.size());
    std::vector<const FunctionType*> old_types(methods.size());
    std::vector<bool> old_nomangle(methods.size());

    for (size_t i = 0; i < methods.size(); i++)
    {
        auto& method = methods[i];

        if (method->template_param_count != -1)
            continue;

        old_names[i] = std::move(method->name);
        method->name = full_name + "." + old_names[i];

        old_types[i] = method->function_type;
        auto new_param_types = old_types[i]->param_types;
        // The first element is a placeholder
        new_param_types[0] =create_type<ReferenceType>(cls);
        auto ret = old_types[i]->return_type;
        auto is_var_arg = old_types[i]->is_var_arg;

        method->function_type = create_type<FunctionType>(ret, new_param_types, is_var_arg);

        old_nomangle[i] = method->nomangle;

        method->set_full_name();
    }

    // Add field constructor args of the concrete class
    // We do this after having the types (and the full names)
    //  of methods updated, and before evaluating the methods
    auto& constructors = templated_class_field_constructor_args[key];
    for (auto& constructor : constructors) {
        name_resolver.add_fields_constructor_args(constructor.func->name, constructor.args);
    }

    std::swap(templated.node->name, full_name);
    templated.node->is_templated = false;
    templated.node->eval();
    templated.node->is_templated = true;
    std::swap(templated.node->name, full_name);

    for (size_t i = 0; i < methods.size(); i++) {
        auto& method = methods[i];
        method->name = std::move(old_names[i]);
        method->function_type = old_types[i];
        method->nomangle = old_nomangle[i];
    }

    current_class = old_class;
    current_function = old_func;

    typing_system.pop_scope();

    typing_system.identifier_types.restore_prev_state();
    name_resolver.symbol_table.restore_prev_state();

    stop_caching_types = old_type_cache_config;
    templated_definition_depth--;

    return cls;
}

bool ModuleCompiler::has_templated_function(const std::string &name) {
    return templated_functions.find(name) != templated_functions.end();
}

bool ModuleCompiler::has_templated_class(const std::string &name) {
    return templated_classes.find(name) != templated_classes.end();
}

}
