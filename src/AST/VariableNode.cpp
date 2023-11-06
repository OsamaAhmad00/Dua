#include <AST/terminals/VariableNode.h>

llvm::Value* VariableNode::eval()
{
    auto variable = symbol_table().contains(name) ?
            symbol_table().get(name) : symbol_table().get_global(name);
    llvm::Value* result = variable.ptr;
    if (dereference)
        result = builder().CreateLoad(variable.type, result);
    return result;
}
