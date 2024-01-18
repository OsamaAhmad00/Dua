#include <resolution/TemplatedNameResolver.hpp>
#include <types/ClassType.hpp>
#include <Value.hpp>
#include <AST/function/FunctionDefinitionNode.hpp>
#include <AST/class/ClassDefinitionNode.hpp>
#include <resolution/NameResolver.hpp>
#include <types/ReferenceType.hpp>
#include <parsing/ParserAssistant.hpp>

namespace dua
{

void check_template_params(const std::vector<std::string>& template_params, const std::string& description = "")
{
    for (size_t i = 0; i < template_params.size(); i++) {
        for (size_t j = i + 1; j < template_params.size(); j++) {
            if (template_params[i] == template_params[j]) {
                std::string message = "The template parameter " + template_params[i] + " is repeated";
                if (!description.empty())
                    message += " in " + description;
                message += ". Can't have more than one template parameter with the same name";
                report_error(message);
            }
        }
    }
}

std::string TemplatedNameResolver::get_templated_function_key(std::string name, size_t args_count) {
    // Templated functions are prefixed with a special keyword to avoid
    // ambiguity with non-templated functions (to not have the same prefix)
    return "Templated." + name + "." + std::to_string(args_count);
}

void TemplatedNameResolver::add_templated_function(FunctionDefinitionNode* node, std::vector<std::string> template_params, FunctionInfo info, const std::string& class_name, bool in_templated_class)
{
    auto name = get_templated_function_key(node->name, template_params.size());
    check_template_params(template_params, "the function " + name);
    auto& functions = templated_functions[std::move(name)];
    for (auto& function : functions) {
        if (*function.info.type == *info.type) {
            report_error("Redefinition of the templated function " + node->name + " with the "
                         + std::to_string(template_params.size()) + " template parameters and the signature " +
                         info.type->to_string());
        }
    }
    auto cls = class_name.empty() ? nullptr : llvm::StructType::getTypeByName(*compiler->get_context(), class_name);
    functions.push_back({node, std::move(template_params), std::move(info), cls, in_templated_class });
}

Value TemplatedNameResolver::get_templated_function(const std::string& name, std::vector<const Type *> &template_args, const std::vector<const Type *> &arg_types, bool use_arg_types, bool panic_on_error)
{
    // Templated functions are named as follows:
    //  original_name.template_param_count.param_types_separated_by_dots
    // We need to include the count of the template parameters count to
    //  allow for functions with same signature, but with different number
    //  of template parameters to exist without collision

    auto it = templated_functions.find(get_templated_function_key(name, template_args.size()));

    if (it == templated_functions.end()) {
        if (!panic_on_error) return {};
        report_error("The templated function " + name + " with "
                     + std::to_string(template_args.size()) + " template parameters is not defined");
    }

    // Finding the best overload

    auto& functions = it->second;
    auto& any_overload = functions.front();

    // Global scope. Other scopes such as the scopes of class fields, class type aliases,
    //  and class template args will be pushed later
    compiler->name_resolver.symbol_table.keep_only_last_n_scopes(0, true);
    compiler->typing_system.identifier_types.keep_only_last_n_scopes(0, true);

    if (any_overload.in_templated_class)
    {
        // If one overload belongs to a class, all other overloads will belong to it too.
        compiler->typing_system.push_scope();
        compiler->name_resolver.push_scope();
        auto class_name = any_overload.owner_class->getName().str();
        auto& bindings = templated_class_bindings[class_name];
        for (size_t i = 0; i < bindings.params.size(); i++)
            compiler->typing_system.insert_type(bindings.params[i], bindings.args[i]);
    }

    long long idx = 0;
    if (!use_arg_types) {
        if (functions.size() > 1) {
            if (!panic_on_error) return {};
            report_error("Can't infer the correct overload for the templated function " + name +
                         " without the knowledge of the argument types");
        }
    } else {
        idx = get_winner_templated_function(name, functions, template_args, arg_types);
    }

    auto& templated = functions[idx];

    // A scope that will hold the bindings of the template
    //  parameters to the template arguments, and will
    //  shadow the outer scope types that collide if exists.
    compiler->typing_system.push_scope();
    compiler->name_resolver.push_scope();

    for (size_t i = 0; i < template_args.size(); i++)
        compiler->typing_system.insert_type(templated.template_params[i], template_args[i]);

    std::string full_name;

    if (use_arg_types)
        full_name = get_templated_function_full_name(name, template_args, templated.info.type->param_types);
    else
        full_name = get_templated_function_full_name(name, template_args);

    if (compiler->name_resolver.has_function(full_name))
    {
        auto func = compiler->module.getFunction(full_name);
        auto type = compiler->name_resolver.get_function_no_overloading(full_name).type;

        compiler->typing_system.identifier_types.restore_prev_state();
        compiler->name_resolver.symbol_table.restore_prev_state();

        return compiler->create_value(func, type);
    }

    // Function is not defined yet.
    // Instantiate the function with the provided arguments

    // Types shouldn't be cached during the evaluation of templated functions
    auto old_type_cache_config = compiler->stop_caching_types;
    compiler->stop_caching_types = true;

    // The full name is computed here.
    templated.node->nomangle = true;

    // Temporarily reset the current function to nullptr
    // so that we don't get the nested functions error.
    auto old_func = compiler->current_function;
    compiler->current_function = nullptr;

    // Temporarily set the current class to the owner
    // class so that method definitions are correct
    auto old_class = compiler->current_class;
    // Templated classes will set the current class
    //  before proceeding
    if (!templated.in_templated_class)
        compiler->current_class = templated.owner_class;

    compiler->name_resolver.register_function(full_name, templated.info, true);

    std::swap(templated.node->name, full_name);
    auto old_template_param_count = templated.node->template_param_count;
    templated.node->template_param_count = FunctionDefinitionNode::TEMPLATED_BUT_EVALUATE;
    templated.node->eval();
    templated.node->template_param_count = old_template_param_count;
    std::swap(templated.node->name, full_name);

    compiler->current_class = old_class;
    compiler->current_function = old_func;

    compiler->typing_system.identifier_types.restore_prev_state();
    compiler->name_resolver.symbol_table.restore_prev_state();

    compiler->stop_caching_types = old_type_cache_config;

    auto func = compiler->module.getFunction(full_name);
    auto type = compiler->name_resolver.get_function_no_overloading(full_name).type;

    return compiler->create_value(func, type);
}

Value TemplatedNameResolver::get_templated_function(const std::string& name, std::vector<const Type *> &template_args, bool panic_on_error) {
    return get_templated_function(std::move(name), template_args, std::vector<const Type*>{}, false, panic_on_error);
}

std::string TemplatedNameResolver::get_templated_function_full_name(std::string name, const std::vector<const Type *> &template_args)
{
    // The same way the param types are added to the name, the template args are added to the name.
    name = get_templated_function_key(std::move(name), template_args.size());
    if (!template_args.empty())
        name = compiler->name_resolver.get_function_full_name(name, template_args);
    return name;
}

std::string TemplatedNameResolver::get_templated_function_full_name(std::string name, const std::vector<const Type *> &template_args, const std::vector<const Type *> &param_types) {
    name = get_templated_function_full_name(std::move(name), template_args);
    return compiler->name_resolver.get_function_full_name(name, param_types);
}

long long TemplatedNameResolver::get_winner_templated_function(const std::string& name, const std::vector<TemplatedFunctionNode> &functions, const std::vector<const Type*>& template_args, const std::vector<const Type *> &arg_types, bool panic_on_not_found)
{
    std::map<int, std::vector<long long>> scores;

    for (size_t i = 0; i < functions.size(); i++)
    {
        compiler->typing_system.push_scope();

        for (size_t j = 0; j < template_args.size(); j++)
            compiler->typing_system.insert_type(functions[i].template_params[j], template_args[j]);

        auto score = compiler->typing_system.type_list_similarity_score(
                functions[i].info.type->param_types,
                arg_types,
                functions[i].info.type->is_var_arg
        );

        if (score == -1) continue;

        scores[score].emplace_back(i);

        compiler->typing_system.pop_scope();
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

std::string TemplatedNameResolver::get_templated_class_key(std::string name, size_t args_count) {
    return name + "." + std::to_string(args_count);
}

std::string TemplatedNameResolver::get_templated_class_full_name(const std::string& name, const std::vector<const Type*>& template_args) {
    return get_templated_function_full_name(name, template_args);
}

void TemplatedNameResolver::add_templated_class(ClassDefinitionNode* node, std::vector<std::string> template_params, const IdentifierType* parent)
{
    auto name = get_templated_class_key(node->name, template_params.size());
    check_template_params(template_params, "the class " + name);
    if (templated_classes.find(name) != templated_classes.end())
        report_error("Redefinition of the templated class " + node->name + " with " + std::to_string(template_params.size()) + " template parameters");
    templated_classes[std::move(name)] = { node, std::move(template_params), std::move(parent) };
}

const ClassType* TemplatedNameResolver::get_templated_class(const std::string &name, const std::vector<const Type*>& template_args)
{
    auto key = get_templated_class_key(name, template_args.size());
    auto it = templated_classes.find(key);
    if (it == templated_classes.end())
        report_error("The templated class " + name + " with " + std::to_string(template_args.size()) + " template parameters is not defined");

    auto& templated = it->second;

    compiler->name_resolver.symbol_table.keep_only_last_n_scopes(0, true);
    compiler->typing_system.identifier_types.keep_only_last_n_scopes(0, true);

    // A scope that will hold the bindings of the template
    //  parameters to the template arguments, and will
    //  shadow the outer scope types that collide if exists.
    compiler->typing_system.push_scope();
    compiler->name_resolver.push_scope();

    auto concrete_args = template_args;
    for (auto& arg : concrete_args)
        arg = arg->get_concrete_type();

    for (size_t i = 0; i < concrete_args.size(); i++)
        compiler->typing_system.insert_type(templated.template_params[i], concrete_args[i]);

    auto full_name = get_templated_class_full_name(name, concrete_args);

    if (!compiler->name_resolver.has_class(full_name)) {
        register_templated_class(name, concrete_args);
        // define_templated_class(name, concrete_args);
    }

    auto cls = compiler->name_resolver.get_class(full_name);

    compiler->typing_system.identifier_types.restore_prev_state();
    compiler->name_resolver.symbol_table.restore_prev_state();

    return cls;
}

void TemplatedNameResolver::add_templated_class_method_info(const std::string &cls, FunctionDefinitionNode* method, FunctionInfo info, std::vector<std::string> template_params) {
    // TODO compute the key as the hash of the triple { name, template param count, type pointer }
    auto key = cls + "." + method->name;
    check_template_params(template_params, "the method " + key);
    if (method->template_param_count != FunctionDefinitionNode::NOT_TEMPLATED)
        key += "." + std::to_string(method->template_param_count);
    key += "." + info.type->as_key();
    templated_class_method_info[key] = { std::move(info), std::move(template_params) };
}

TemplatedClassMethodInfo TemplatedNameResolver::get_templated_class_method_info(const std::string &cls, const std::string &method, const FunctionType* type, size_t template_param_count) {
    auto key = cls + "." + method;
    if (template_param_count != -1)
        key += "." + std::to_string(template_param_count);
    key += "." + type->as_key();
    auto it = templated_class_method_info.find(key);
    if (it == templated_class_method_info.end())
        report_internal_error("There is no info stored for the method " + method + " of the templated class " + cls);
    return it->second;
}

void TemplatedNameResolver::register_templated_class(const std::string &name, const std::vector<const Type *> &template_args)
{
    auto key = get_templated_class_key(name, template_args.size());
    auto it = templated_classes.find(key);
    if (it == templated_classes.end())
        report_error("The templated class " + name + " with " + std::to_string(template_args.size()) + " template parameters is not defined");

    auto& templated = it->second;

    // Registering the methods of the class

    compiler->name_resolver.symbol_table.keep_only_last_n_scopes(0, true);
    compiler->typing_system.identifier_types.keep_only_last_n_scopes(0, true);

    // A scope that will hold the bindings of the template
    //  parameters to the template arguments, and will
    //  shadow the outer scope types that collide if exists.
    compiler->typing_system.push_scope();
    compiler->name_resolver.push_scope();

    auto concrete_args = template_args;
    for (auto& arg : concrete_args)
        arg = arg->get_concrete_type();

    for (size_t i = 0; i < template_args.size(); i++)
        compiler->typing_system.insert_type(templated.template_params[i], concrete_args[i]);

    auto full_name = get_templated_class_full_name(name, concrete_args);

    if (compiler->name_resolver.has_function(full_name))
        report_error("There is already a function with the name " + full_name + ". Can't have a class with the same name");

    auto cls = compiler->create_type<ClassType>(full_name);
    compiler->name_resolver.classes[full_name] = cls;
    compiler->typing_system.insert_global_type(full_name, cls);

    templated_class_bindings[full_name] = { templated.template_params, concrete_args };

    auto parent = it->second.parent;
    compiler->name_resolver.parent_classes[full_name] = get_parent_class(parent);

    auto& methods = templated.node->methods;
    for (auto & method : methods)
    {
        auto method_name = method->name;
        if (!method->is_operator)
            method_name =  full_name + "." + method_name;

        auto new_param_types = method->function_type->param_types;
        // The first element is a placeholder, and we need to replace it with the instance type
        new_param_types[0] = compiler->create_type<ReferenceType>(cls, true);
        auto ret = method->function_type->return_type;
        auto is_var_arg = method->function_type->is_var_arg;

        auto concrete_type = compiler->create_type<FunctionType>(ret, new_param_types, is_var_arg);

        auto [info, template_params] = get_templated_class_method_info(key, method->name, method->function_type, method->template_param_count);
        info.type = concrete_type;

        if (method->template_param_count != FunctionDefinitionNode::NOT_TEMPLATED) {
            // Cloning the method node here because we're going to restore its function type later
            auto node = compiler->create_node<FunctionDefinitionNode>(method_name, method->body, concrete_type, method->nomangle, method->template_param_count);
            add_templated_function(node, std::move(template_params), std::move(info), full_name, true);
        } else {
            // Setting the full name after the substitution of the concrete class type
            auto full_method_name = compiler->name_resolver.get_function_full_name(method_name, concrete_type->param_types);
            compiler->name_resolver.register_function(std::move(full_method_name), std::move(info), true);
        }
    }

    compiler->typing_system.identifier_types.restore_prev_state();
    compiler->name_resolver.symbol_table.restore_prev_state();
}

const ClassType* TemplatedNameResolver::define_templated_class(const std::string &name, const std::vector<const Type *> &template_args)
{
    auto key = get_templated_class_key(name, template_args.size());
    auto it = templated_classes.find(key);
    if (it == templated_classes.end())
        report_error("The templated class " + name + " with " + std::to_string(template_args.size()) + " template parameters is not defined");

    auto& templated = it->second;

    compiler->name_resolver.symbol_table.keep_only_last_n_scopes(0, true);
    compiler->typing_system.identifier_types.keep_only_last_n_scopes(0, true);

    // A scope that will hold the bindings of the template
    //  parameters to the template arguments, and will
    //  shadow the outer scope types that collide if exists.
    compiler->typing_system.push_scope();
    compiler->name_resolver.push_scope();

    for (size_t i = 0; i < template_args.size(); i++)
        compiler->typing_system.insert_type(templated.template_params[i], template_args[i]);

    auto full_name = get_templated_class_full_name(name, template_args);

    // Class is not defined yet.
    // Instantiate the class with the provided arguments

    // Types shouldn't be cached during the evaluation of templated classes
    auto old_type_cache_config = compiler->stop_caching_types;
    compiler->stop_caching_types = true;

    // Temporarily set the current function to nullptr
    // so that we don't get the nested functions error.
    auto old_func = compiler->current_function;
    compiler->current_function = nullptr;

    // Temporarily set the current class to nullptr so
    // that we don't get the nested class definition error.
    auto old_class = compiler->current_class;
    compiler->current_class = nullptr;

    auto cls = compiler->name_resolver.get_class(full_name);

    // Making a copy to modify it freely
    auto class_node = *templated.node;

    std::vector<FunctionDefinitionNode> method_copies;
    for (auto & method : class_node.methods)
        method_copies.push_back(*method);

    // It's important that we set it to the addresses after
    //  pushing back all the nodes, because a vector may allocate
    //  new memory location if the current is not enough
    for (size_t i = 0; i < method_copies.size(); i++)
        class_node.methods[i] = &method_copies[i];

    auto& methods = class_node.methods;
    auto& constructors = templated_class_field_constructor_args[key];

    size_t constructors_count = 0;
    for (size_t i = 0; i < methods.size(); i++) {
        // Put the constructor nodes at the front
        bool is_constructor = methods[i]->name == "constructor" || methods[i]->name == "=constructor";
        if (is_constructor) {
            // Put the constructor at the same place as the constructors in the constructors vector
            for (size_t j = 0; j < constructors.size(); j++) {
                if (templated.node->methods[i] == constructors[j].func && i != j) {
                    // This will put the constructors at the same correct position in the
                    //  templated node too, so that we don't need to do this on the next
                    //  templated class instantiation
                    std::swap(templated.node->methods[i], templated.node->methods[j]);
                    std::swap(methods[i--], methods[j]);
                    constructors_count--;
                    break;
                }
            }
            constructors_count++;
        }
    }

    assert(constructors_count == constructors.size());

    for (size_t i = 0; i < methods.size(); i++)
    {
        auto& method = methods[i];

        if (method->template_param_count != FunctionDefinitionNode::NOT_TEMPLATED)
            continue;

        if (!method->is_operator)
            method->name =  full_name + "." + method->name;

        auto new_param_types = method->function_type->param_types;
        // The first element is a placeholder, and we need to replace it with the self type
        new_param_types[0] =compiler->create_type<ReferenceType>(cls, true);
        auto ret = method->function_type->return_type;
        auto is_var_arg = method->function_type->is_var_arg;

        method->function_type = compiler->create_type<FunctionType>(ret, std::move(new_param_types), is_var_arg);

        method->set_full_name();

        if (i < constructors_count) {
            // Add field constructor args of the concrete class
            // We do this after having the types (and the full names)
            //  of methods updated, and before evaluating the methods
            compiler->name_resolver.add_fields_constructor_args(method->name, constructors[i].args);
        }
    }

    class_node.name = full_name;
    class_node.is_templated = false;
    class_node.eval();

    compiler->current_class = old_class;
    compiler->current_function = old_func;

    compiler->typing_system.identifier_types.restore_prev_state();
    compiler->name_resolver.symbol_table.restore_prev_state();

    compiler->stop_caching_types = old_type_cache_config;

    return cls;
}

bool TemplatedNameResolver::has_templated_function(const std::string &name) {
    return templated_functions.find(name) != templated_functions.end();
}

bool TemplatedNameResolver::has_templated_class(const std::string &name) {
    return templated_classes.find(name) != templated_classes.end();
}

const ClassType *TemplatedNameResolver::get_parent_class(const IdentifierType* parent)
{
    // Parent classes are processes after all classes (including templated ones) are defined,
    //  and all global aliases are defined as well.

    if (parent->is_templated) {
        auto template_args = parent->template_args;
        for (auto& type : template_args)
            type = type->get_concrete_type();
        auto full_name = get_templated_class_full_name(parent->name, template_args);
        if (!compiler->name_resolver.has_class(full_name)) {
            // The templated parent class is not registered yet.
            // This is the correct place to define it at, because
            //  its template args may be a template parameter
            //  of the child, which are bound only at the moment
            // The fact that it's not registered at this point, means
            //  that it won't be used except as a parent to this class
            //  and maybe for some other classes that are not defined yet.
            //  This means that we can just both register it and define
            //  it here with no problem.
            register_templated_class(parent->name, template_args);
            // define_templated_class(parent->name, template_args);

            // FIXME The classes are tightly coupled. Find another way to register this information
            // This is the only place in which all information are present. This is a quick dirty
            //  hack to register the info of the parent class.
            auto& info = compiler->parser_assistant->class_info[full_name];  // Using the full name to avoid collisions
            if (!info.name.empty())
                report_error("Redefinition of the templated class " + parent->name + " with " + std::to_string(template_args.size()) + " template parameters");
            info.template_args = template_args;
            info.is_templated = true;
            info.name = parent->name;
        }
        return compiler->name_resolver.get_class(full_name);
    } else {
        auto parent_type = compiler->typing_system.get_type(parent->name);
        auto concrete_class = parent_type->get_concrete_type()->as<ClassType>();
        if (concrete_class == nullptr)
            report_error("The type " + parent_type->to_string() + " is not a class type, and can't be a parent class");
        auto& classes = compiler->name_resolver.classes;
        if (classes.find(concrete_class->name) == classes.end())
            report_error("The parent class " + parent->name + " is not defined");
        return concrete_class;
    }
}

}
