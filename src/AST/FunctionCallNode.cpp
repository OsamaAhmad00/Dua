#include <AST/function/FunctionCallNode.hpp>
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"
#include <AST/function/FunctionDefinitionNode.hpp>


namespace dua
{

FunctionCallNode::FunctionCallNode(ModuleCompiler *compiler, std::string name, std::vector<ASTNode *> args,
                                   std::vector<const Type*> template_args)
        : name(std::move(name)), args(std::move(args)), template_args(std::move(template_args)), is_templated(true)
{
    this->compiler = compiler;
}

FunctionCallNode::FunctionCallNode(ModuleCompiler *compiler, std::string name, std::vector<ASTNode *> args)
    : name(std::move(name)), args(std::move(args)), template_args({}), is_templated(false)
{
    this->compiler = compiler;
}

std::vector<Value> FunctionCallNode::eval_args()
{
    auto n = args.size();
    std::vector<Value> evaluated(n);
    for (size_t i = 0; i < n; i++)
        evaluated[i] = args[i]->eval();
    return evaluated;
}

Value FunctionCallNode::eval()
{
    auto evaluated_args = eval_args();

    if (is_templated)
        return call_templated_function(std::move(evaluated_args));

    if (name_resolver().has_function(name))
        return name_resolver().call_function(name, std::move(evaluated_args));

    if (!name_resolver().symbol_table.contains(name))
        compiler->report_error("The identifier " + name + " doesn't refer to either a function or a function reference");

    auto reference = name_resolver().symbol_table.get(name);

    return call_reference(reference, std::move(evaluated_args));
}

const Type *FunctionCallNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    std::vector<const Type*> arg_types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        arg_types[i] = args[i]->get_type();

    if (is_templated) {
        auto templated = get_templated_function(arg_types);
        if (!templated.is_null())
            return set_type(templated.type->as<FunctionType>()->return_type);
    }

    return set_type(name_resolver().get_function(name, arg_types).type->return_type);
}

Value FunctionCallNode::get_method(std::vector<const Type*>& arg_types)
{
    if (current_class() == nullptr)
        return {};

    auto class_name = current_class()->getName().str();
    auto class_type = name_resolver().get_class(class_name);
    auto reference_type = compiler->create_type<ReferenceType>(class_type, true);
    arg_types.insert(arg_types.begin(), reference_type);
    auto method = name_resolver().get_templated_function(class_name + "." + name, template_args, arg_types, true, false);
    if (!method.is_null()) {
        return method;
    } else {
        // Undo
        arg_types.erase(arg_types.begin());
        return {};
    }
}

Value FunctionCallNode::call_templated_function(std::vector<Value> args)
{
    std::vector<const Type*> arg_types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        arg_types[i] = args[i].type;

    auto func = get_templated_function(arg_types);
    if (arg_types.size() == args.size() + 1) {
        // This is a method call
        auto self = name_resolver().symbol_table.get("self");
        args.insert(args.begin(), self);
    }
    return name_resolver().call_function(func, std::move(args));
}

Value FunctionCallNode::call_reference(const Value &reference, std::vector<Value> args) {

    auto pointer_type = dynamic_cast<const PointerType*>(reference.type);
    if (!pointer_type) {
        if (auto c = reference.type->get_contained_type(); c != nullptr)
            pointer_type = c->as<PointerType>();
    }

    auto function_type = pointer_type ? dynamic_cast<const FunctionType*>(pointer_type->get_element_type()) : nullptr;
    if (function_type == nullptr)
        compiler->report_error("The variable " + name + " is of type " + reference.type->to_string() + ", which is not callable");

    auto ptr = builder().CreateLoad(reference.type->llvm_type(), reference.get());
    auto value = compiler->create_value(ptr, function_type);

    return name_resolver().call_function(value, std::move(args));
}

Value FunctionCallNode::get_templated_function(std::vector<const Type*>& arg_types)
{
    // Try as a method first
    auto method = get_method(arg_types);
    if (!method.is_null())
        return method;

    return compiler->get_name_resolver().get_templated_function(name, template_args, std::move(arg_types));
}

}