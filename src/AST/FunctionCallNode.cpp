#include <AST/function/FunctionCallNode.hpp>
#include "types/PointerType.hpp"


namespace dua
{

llvm::CallInst *FunctionCallNode::eval()
{
    std::vector<Value> evaluated(args.size());
    for (size_t i = 0; i < args.size(); i++)
        evaluated[i] = compiler->create_value(args[i]->eval(), args[i]->get_type());

    if (name_resolver().has_function(name))
        return name_resolver().call_function(name, std::move(evaluated));

    // It has to be a function reference.
    auto reference = compiler->create_value((llvm::Value*)nullptr, (const Type*)nullptr);
    if (name_resolver().symbol_table.contains(name))
        reference = name_resolver().symbol_table.get(name);
    else if (!evaluated.empty()) {
        // It might be a class field
        auto& instance = evaluated[0];
        auto pointer_type = dynamic_cast<const PointerType*>(instance.type);
        auto class_type = pointer_type ? dynamic_cast<const ClassType*>(pointer_type->get_element_type()) : nullptr;
        if (class_type != nullptr && class_type->name == name.substr(0, class_type->name.size())) {
            auto field_name = name.substr(class_type->name.size() + 1);
            auto field = class_type->get_field(field_name);
            reference.type = field.type;
            reference.ptr = class_type->get_field(instance.ptr, field_name);
            // It's a field that points to a function. The instance shouldn't be passed
            evaluated.erase(evaluated.begin());
        }
    }

    if (reference.ptr == nullptr)
        report_error("The identifier " + name + " doesn't refer to either a function or a function reference");

    auto pointer_type = dynamic_cast<const PointerType*>(reference.type);
    auto function_type = pointer_type ? dynamic_cast<const FunctionType*>(pointer_type->get_element_type()) : nullptr;
    if (function_type == nullptr)
        report_error("The variable " + name + " is of type " + reference.type->to_string() + ", which is not callable");

    auto ptr = builder().CreateLoad(reference.type->llvm_type(), reference.ptr);

    return name_resolver().call_function(ptr, function_type, std::move(evaluated));
}

const Type *FunctionCallNode::get_type()
{
    if (type != nullptr) return type;

    std::vector<const Type*> arg_types(args.size());
    for (size_t i = 0; i < args.size(); i++)
        arg_types[i] = args[i]->get_type();
    return type = name_resolver().get_function(name, arg_types).type->return_type;
}

}