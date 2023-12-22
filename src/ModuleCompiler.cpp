#include <ModuleCompiler.hpp>
#include <parsing/ParserFacade.hpp>
#include <AST/TranslationUnitNode.hpp>
#include <llvm/Support/Host.h>
#include "AST/function/FunctionDefinitionNode.hpp"

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

void ModuleCompiler::add_templated_function(FunctionDefinitionNode* node, std::vector<std::string> template_params, FunctionInfo info, llvm::StructType* current_class)
{
    auto name = node->name + "." + std::to_string(template_params.size());
    if (auto it = templated_functions.find(name); it == templated_functions.end()) {
        templated_functions[std::move(name)] = {node, std::move(template_params), std::move(info), current_class};
    } else {
        report_error("The templated function " + node->name + " with "
                     + std::to_string(template_params.size()) + " template parameters is already defined");
    }
}

Value ModuleCompiler::get_templated_function(std::string name, std::vector<const Type *> &template_args,
                                                 const std::vector<const Type *> &arg_types, bool use_arg_types)
{
    // Templated functions are named as follows:
    //  original_name.template_param_count.param_types_separated_by_dots
    // We need to include the count of the template parameters count to
    //  allow for functions with same signature, but with different number
    //  of template parameters to exist without collision

    auto it = templated_functions.find(name + "." + std::to_string(template_args.size()));

    if (it == templated_functions.end())
        report_error("The templated function " + name + " with "
                     + std::to_string(template_args.size()) + " template parameters is not defined");

    std::string full_name;

    if (use_arg_types)
        full_name = get_templated_function_full_name(std::move(name), template_args, arg_types);
    else
        full_name = get_templated_function_full_name(std::move(name), template_args);

    if (name_resolver.has_function(full_name)) {
        auto func = module.getFunction(full_name);
        auto type = name_resolver.get_function_no_overloading(full_name).type;
        return create_value(func, type);
    }

    // Function not defined yet.
    // Instantiate the function with the provided arguments

    auto& templated = it->second;

    // The full name is computed here.
    templated.node->no_mangle = true;

    name_resolver.symbol_table.temporarily_discard_all_but_global_scope();
    typing_system.identifier_types.temporarily_discard_all_but_global_scope();

    // A scope that will hold the binding of the template
    //  parameters to the template arguments, and will
    //  shadow the outer scope types that collide if exists.
    typing_system.push_scope();

    for (size_t i = 0; i < template_args.size(); i++)
        typing_system.insert_type(templated.template_params[i], template_args[i]);

    // Temporarily reset set the current function to nullptr
    // so that we don't get the nested functions error.
    auto old_func = current_function;
    current_function = nullptr;

    // Temporarily reset set the current class to
    // nullptr so that method definitions are correct
    auto old_class = current_class;
    current_class = templated.owner_class;

    name_resolver.register_function(full_name, templated.info, true);

    std::swap(templated.node->name, full_name);
    templated.node->is_templated = false;
    templated.node->eval();
    templated.node->is_templated = true;
    std::swap(templated.node->name, full_name);

    current_class = old_class;
    current_function = old_func;

    typing_system.pop_scope();

    typing_system.identifier_types.restore_original_scopes();
    name_resolver.symbol_table.restore_original_scopes();

    auto func = module.getFunction(full_name);
    auto type = name_resolver.get_function_no_overloading(full_name).type;

    return create_value(func, type);
}

Value ModuleCompiler::get_templated_function(std::string name, std::vector<const Type *> &template_args) {
    return get_templated_function(std::move(name), template_args, std::vector<const Type*>{}, false);
}

std::string ModuleCompiler::get_templated_function_full_name(std::string name, const std::vector<const Type *> &template_args)
{
    // The same way the param types are added to the name, the template args are added to the name.
    name += "." + std::to_string(template_args.size());
    if (!template_args.empty())
        name = name_resolver.get_function_full_name(name, template_args);
    return name;
}

std::string ModuleCompiler::get_templated_function_full_name(std::string name, const std::vector<const Type *> &template_args, const std::vector<const Type *> &param_types) {
    name = get_templated_function_full_name(std::move(name), template_args);
    return name_resolver.get_function_full_name(name, param_types);
}

}
