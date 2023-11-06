#include <AST/GlobalVariableDefinitionNode.h>

llvm::GlobalVariable* GlobalVariableDefinitionNode::eval()
{
    module().getOrInsertGlobal(name, initializer->get_type()->llvm_type());
    llvm::GlobalVariable* variable = module().getGlobalVariable(name);
    variable->setConstant(false);
    // Should any variable type be aligned this way?
    variable->setAlignment(llvm::MaybeAlign(4));
    variable->setExternallyInitialized(false);
    symbol_table().insert_global(name, { variable, initializer->get_type()->llvm_type() });
    variable->setInitializer(initializer->eval());
    return variable;
}

GlobalVariableDefinitionNode::~GlobalVariableDefinitionNode()
{
    delete initializer;
}