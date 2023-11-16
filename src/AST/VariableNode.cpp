#include "AST/lvalue/VariableNode.h"

namespace dua
{

llvm::Value* VariableNode::eval()
{
    // This searches locally first, then globally if not found.
    auto variable = symbol_table().get(name);

    if (!load_value)
        return variable.ptr;

    return builder().CreateLoad(variable.type->llvm_type(), variable.ptr);
}

TypeBase* VariableNode::compute_type() {
    return type = symbol_table().get(name).type;
}

}
