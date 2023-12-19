#include <AST/function/FunctionCallNode.hpp>
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"
#include <AST/function/FunctionDefinitionNode.hpp>


namespace dua
{

FunctionCallNode::FunctionCallNode(ModuleCompiler *compiler, std::string name, std::vector<ASTNode *> args,
                                   std::vector<const Type*> template_args)
        : name(name), args(std::move(args)), template_args(template_args)
{
    this->compiler = compiler;
}

std::vector<Value> FunctionCallNode::eval_args(bool is_method)
{
    auto n = args.size() - is_method;
    std::vector<Value> evaluated(n);
    for (size_t i = 0; i < args.size() - is_method; i++)
        evaluated[i] = args[i + is_method]->eval();
    return evaluated;
}

Value FunctionCallNode::eval()
{
    if (!template_args.empty())
        return call_templated_function();

    std::vector<const Type*> arg_types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        arg_types[i] = args[i]->get_type();

    if (name_resolver().has_function(name))
        return name_resolver().call_function(name, eval_args());

    // It has to be a function reference.
    bool is_field = false;
    Value reference;
    if (name_resolver().symbol_table.contains(name))
        reference = name_resolver().symbol_table.get(name);
    else if (!args.empty()) {
        // It might be a class field, called within a method in the class, in which
        //  case, the first argument will be the instance itself
        auto instance_type = dynamic_cast<const ReferenceType*>(args[0]->get_type());
        auto class_type = instance_type ? instance_type->get_element_type()->as<ClassType>() : nullptr;
        if (class_type != nullptr && class_type->name == name.substr(0, class_type->name.size())) {
            auto field_name = name.substr(class_type->name.size() + 1);  // + 1, ignoring the dot
            auto field = class_type->get_field(field_name);
            reference.type = field.type;
            reference.set(class_type->get_field(args[0]->eval(), field_name).get());
            is_field = true;
        }
    }

    if (reference.is_null())
        report_error("The identifier " + name + " doesn't refer to either a function or a function reference");

    auto pointer_type = dynamic_cast<const PointerType*>(reference.type);
    if (!pointer_type) {
        if (auto c = reference.type->get_contained_type(); c != nullptr)
            pointer_type = c->as<PointerType>();
    }
    auto function_type = pointer_type ? dynamic_cast<const FunctionType*>(pointer_type->get_element_type()) : nullptr;
    if (function_type == nullptr)
        report_error("The variable " + name + " is of type " + reference.type->to_string() + ", which is not callable");

    auto ptr = builder().CreateLoad(reference.type->llvm_type(), reference.get());
    auto value = compiler->create_value(ptr, function_type);

    return name_resolver().call_function(value, eval_args(is_field));
}

const Type *FunctionCallNode::get_type()
{
    if (type != nullptr) return type;

    std::vector<const Type*> arg_types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        arg_types[i] = args[i]->get_type();
    return type = name_resolver().get_function(name, arg_types).type->return_type;
}

Value FunctionCallNode::call_templated_function()
{
    auto full_name = name_resolver().get_templated_function_full_name(name, template_args);
    if (name_resolver().has_function(full_name))
        return name_resolver().call_function(full_name, eval_args());

    // Instantiate the function with the provided arguments

    // A scope that will hold the binding of the template
    //  parameters to the template arguments, and will
    //  shadow the outer scope types that collide if exists.
    typing_system().push_scope();

    auto& templated = name_resolver().get_templated_function(name, template_args.size());
    for (size_t i = 0; i < template_args.size(); i++)
        typing_system().insert_type(templated.template_params[i], template_args[i]);

    // Temporarily reset set the current function to nullptr
    // so that we don't get the nested functions error.
    auto old_func = current_function();
    current_function() = nullptr;

    name_resolver().register_function(full_name, templated.info, true);

    auto func_node = (FunctionDefinitionNode*)templated.node;
    func_node->is_templated = false;
    std::swap(func_node->name, full_name);
    templated.node->eval();
    std::swap(func_node->name, full_name);
    func_node->is_templated = true;

    current_function() = old_func;

    typing_system().pop_scope();

    return name_resolver().call_function(full_name, eval_args());
}

}