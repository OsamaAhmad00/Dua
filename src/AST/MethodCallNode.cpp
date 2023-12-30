#include <AST/function/MethodCallNode.hpp>
#include <types/ReferenceType.hpp>
#include <AST/lvalue/VariableNode.hpp>

namespace dua
{

void MethodCallNode::process()
{
    if (processed)
        return;

    auto instance = name_resolver().symbol_table.get(instance_name);
    auto class_type = instance.type->as<ClassType>();
    name = class_type->name + "." + name;
    auto ref_type = compiler->create_type<ReferenceType>(class_type);
    args.insert(args.begin(), compiler->create_node<VariableNode>(instance_name, ref_type));
    processed = true;
}

Value MethodCallNode::eval()
{
    process();
    return FunctionCallNode::eval();
}

const Type* MethodCallNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    process();

    return set_type(FunctionCallNode::get_type());
}

}