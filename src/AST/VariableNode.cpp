#include <AST/lvalue/VariableNode.hpp>
#include <types/PointerType.hpp>

namespace dua
{

llvm::Value* VariableNode::eval()
{
    if (name_resolver().symbol_table.contains(name)) {
        // This searches locally first, then globally if not found.
        return name_resolver().symbol_table.get(name).ptr;
    }

    // Not found. Has to be a function reference
    auto function = name_resolver().get_function(name);
    return module().getFunction(name);
}

const Type* VariableNode::get_type()
{
    if (type != nullptr) return type;
    const Type* t;
    if (name_resolver().symbol_table.contains(name)) {
        t = name_resolver().symbol_table.get(name).type;
    } else if (name_resolver().has_function(name)) {
        auto full_name = name_resolver().get_function(name);
        t = name_resolver().get_function_no_overloading(full_name).type;
    } else {
        report_error("The identifier " + name + " is not defined");
    }
    return type = compiler->create_type<PointerType>(t);
}

const Type *VariableNode::get_element_type()
{
    return ((PointerType*)get_type())->get_element_type();
}

bool VariableNode::is_function() const {
    return name_resolver().has_function(name);
}

}
