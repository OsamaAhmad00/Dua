#include <AST/function/FunctionCallNode.hpp>
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"
#include "types/VoidType.hpp"
#include <AST/function/FunctionDefinitionNode.hpp>


namespace dua
{

FunctionCallNode::FunctionCallNode(ModuleCompiler *compiler, ResolutionString* name, std::vector<ASTNode *> args,
                                   std::vector<const Type*> template_args)
   : FunctionCallBase(compiler, std::move(args)), unresolved_name(name), template_args(std::move(template_args)), is_templated(true)
{ }

FunctionCallNode::FunctionCallNode(ModuleCompiler *compiler, ResolutionString* name, std::vector<ASTNode*> args)
    : FunctionCallBase(compiler, std::move(args)), unresolved_name(name), template_args({}), is_templated(false)
{ }

Value FunctionCallNode::eval()
{
    _current_name = unresolved_name->resolve();

    if (_current_name == "printf") {
        _current_name = "printf";
    }

    auto evaluated_args = eval_args(args);

    if (is_templated)
        return call_templated_function(std::move(evaluated_args));

    if (current_class() != nullptr)
    {
        // Try as method first
        auto class_name = current_class()->getName().str();
        auto class_type = name_resolver().get_class(class_name);
        std::vector<const Type*> arg_types(evaluated_args.size() + 1);
        for (int i = 0; i < evaluated_args.size(); i++)
            arg_types[i + 1] = evaluated_args[i].type;
        arg_types[0] = compiler->create_type<ReferenceType>(class_type, true);
        auto instance = name_resolver().symbol_table.get("self");
        // This is always an unallocated reference type, but since the symbol table stores addresses
        //  in the loaded_value field, we need to move it back again to memory_location.
        instance.memory_location = instance.get();
        instance.set(nullptr);
        auto method = class_type->get_method(_current_name, instance, std::move(arg_types), false);
        if (!method.is_null()) {
            evaluated_args.insert(evaluated_args.begin(), instance);
            return name_resolver().call_function(method, std::move(evaluated_args));
        }
    }

    if (name_resolver().has_function(_current_name, false))
    {
        std::vector<const Type*> arg_types(evaluated_args.size());
        for (size_t i = 0; i < arg_types.size(); i++)
            arg_types[i] = evaluated_args[i].type;
        auto full_name = name_resolver().get_winning_function(_current_name, arg_types);
        auto info = name_resolver().get_function_no_overloading(full_name);
        auto func = module().getFunction(full_name);
        auto func_value = compiler->create_value(func, info.type);
        return name_resolver().call_function(func_value, std::move(evaluated_args));
    }

    if (!name_resolver().symbol_table.contains(_current_name)) {
        compiler->report_error("The identifier " + _current_name + " doesn't refer to either a function or"
                           " a function reference. If you meant to instantiate an object of type " + _current_name +
                           ", remember that the class name comes after the argument list (e.g. (...)" + _current_name + ")");
    }

    auto reference = name_resolver().symbol_table.get(_current_name);

    return call_reference(reference, std::move(evaluated_args));
}

const Type *FunctionCallNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    _current_name = unresolved_name->resolve();

    std::vector<const Type*> arg_types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        arg_types[i] = args[i]->get_type();

    if (is_templated) {
        auto templated = get_templated_function(arg_types);
        if (!templated.is_null())
            return set_type(templated.type->as<FunctionType>()->return_type);
    }

    if (current_class() != nullptr)
    {
        // Try as method first

        if (_current_name == "constructor")
            return set_type(compiler->create_type<VoidType>());

        auto class_name = current_class()->getName().str();
        auto vtable = name_resolver().get_vtable_instance(class_name);
        auto method_name = name_resolver().get_function_full_name(_current_name, arg_types);
        auto full_name = vtable->method_names_without_class_prefix.find(method_name);
        if (full_name != vtable->method_names_without_class_prefix.end()) {
            auto info = name_resolver().get_function_no_overloading(full_name->second);
            return set_type(info.type->return_type);
        }
    }

    return set_type(name_resolver().get_function(_current_name, arg_types).type->return_type);
}

Value FunctionCallNode::call_templated_function(std::vector<Value> args)
{
    if (current_class() != nullptr)
    {
        // Try as method first
        auto self = name_resolver().symbol_table.get("self");
        self.memory_location = self.get();
        self.set(nullptr);
        std::vector<Value> method_args(args.size() + 1);
        method_args[0] = self;
        for (size_t i = 0; i < args.size(); i++)
            method_args[i + 1] = args[i];
        auto class_name = current_class()->getName().str();
        auto result = compiler->get_name_resolver().call_templated_function(class_name + "." + _current_name, template_args, std::move(method_args), false);
        if (!result.is_null())
            return result;
    }

    return name_resolver().call_templated_function(_current_name, template_args, std::move(args));
}

Value FunctionCallNode::call_reference(const Value &reference, std::vector<Value> args)
{
    auto pointer_type = dynamic_cast<const PointerType*>(reference.type);
    if (!pointer_type) {
        if (auto c = reference.type->get_contained_type(); c != nullptr)
            pointer_type = c->as<PointerType>();
    }

    auto function_type = pointer_type ? dynamic_cast<const FunctionType*>(pointer_type->get_element_type()) : nullptr;
    if (function_type == nullptr)
        compiler->report_error("The variable " + _current_name + " is of type " + reference.type->to_string() + ", which is not callable");

    auto ptr = builder().CreateLoad(reference.type->llvm_type(), reference.get());
    auto value = compiler->create_value(ptr, function_type);

    return name_resolver().call_function(value, std::move(args));
}

Value FunctionCallNode::get_templated_function(std::vector<const Type*>& arg_types)
{
    if (current_class() != nullptr)
    {
        // Try as a method first
        auto class_name = current_class()->getName().str();
        auto class_type = name_resolver().get_class(class_name);
        auto reference_type = compiler->create_type<ReferenceType>(class_type, true);
        arg_types.insert(arg_types.begin(), reference_type);
        auto method = name_resolver().get_templated_function(class_name + "." + _current_name, template_args, arg_types, true, false);
        if (!method.is_null()) {
            return method;
        } else {
            // Undo
            arg_types.erase(arg_types.begin());
        }
    }

    return compiler->get_name_resolver().get_templated_function(_current_name, template_args, arg_types);
}

}