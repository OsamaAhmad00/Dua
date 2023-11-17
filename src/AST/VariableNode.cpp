#include <AST/lvalue/VariableNode.h>
#include <types/PointerType.h>

namespace dua
{

llvm::Value* VariableNode::eval() {
    // This searches locally first, then globally if not found.
    return symbol_table().get(name).ptr;
}

TypeBase* VariableNode::compute_type() {
    delete type;
    return type = compiler->create_type<PointerType>(symbol_table().get(name).type);
}

TypeBase *VariableNode::get_element_type() {
    return symbol_table().get(name).type;
}

}
