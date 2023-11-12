#include <AST/GlobalVariableDefinitionNode.h>

llvm::GlobalVariable* GlobalVariableDefinitionNode::eval()
{
    llvm::Value* value = initializer->eval();
    value = compiler->cast_value(value, type->llvm_type());
    if (value == nullptr)
        throw std::runtime_error("Type mismatch");
    module().getOrInsertGlobal(name, type->llvm_type());
    llvm::GlobalVariable* variable = module().getGlobalVariable(name);
    variable->setConstant(false);
    // Should any variable type be aligned this way?
    variable->setAlignment(llvm::MaybeAlign(4));
    variable->setExternallyInitialized(false);
    symbol_table().insert_global(name, { variable, initializer->get_type()->llvm_type() });
    variable->setInitializer((llvm::Constant*)value);
    return variable;
}

GlobalVariableDefinitionNode::~GlobalVariableDefinitionNode()
{
    delete initializer;
}