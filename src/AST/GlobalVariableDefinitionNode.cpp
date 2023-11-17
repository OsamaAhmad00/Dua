#include <AST/GlobalVariableDefinitionNode.h>

namespace dua
{

llvm::GlobalVariable* GlobalVariableDefinitionNode::eval()
{
    llvm::Value* value = initializer->eval();

    value = compiler->cast_value(value, type->llvm_type());
    if (value == nullptr)
        throw std::runtime_error("Type mismatch");

    auto constant = llvm::dyn_cast<llvm::Constant>(value);
    if (constant == nullptr)
        throw std::runtime_error("Invalid global variable initializer");

    module().getOrInsertGlobal(name, type->llvm_type());
    llvm::GlobalVariable* variable = module().getGlobalVariable(name);
    variable->setInitializer((llvm::Constant*)value);
    variable->setConstant(false);

    symbol_table().insert_global(name, { variable, initializer->get_cached_type() });

    return variable;
}

GlobalVariableDefinitionNode::~GlobalVariableDefinitionNode()
{
    delete initializer;
}

}