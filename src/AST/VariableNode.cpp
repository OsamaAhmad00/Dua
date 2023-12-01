#include <AST/lvalue/VariableNode.h>
#include <types/PointerType.h>

namespace dua
{

llvm::Value* VariableNode::eval()
{
    if (symbol_table().contains(name)) {
        // This searches locally first, then globally if not found.
        return symbol_table().get(name).ptr;
    }

    // Not found. Has to be a function reference
    if (compiler->has_function(name))
        return module().getFunction(name);

    report_error("Reference to undefined identifier: " + name);
    // Unreachable
    return nullptr;
}

TypeBase* VariableNode::compute_type()
{
    delete type;
    TypeBase* t;
    if (symbol_table().contains(name)) {
        t = symbol_table().get(name).type->clone();
    } else {
        t = compiler->get_function(name).type.clone();
    }
    return type = compiler->create_type<PointerType>(t);
}

TypeBase *VariableNode::get_element_type()
{
    return ((PointerType*)get_cached_type())->get_element_type();
}

bool VariableNode::is_function() const {
    return compiler->has_function(name);
}

}
