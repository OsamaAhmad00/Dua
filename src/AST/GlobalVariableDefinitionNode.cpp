#include <AST/GlobalVariableDefinitionNode.h>
#include <utils/ErrorReporting.h>

namespace dua
{

llvm::GlobalVariable* GlobalVariableDefinitionNode::eval()
{
    llvm::Value* value = (initializer != nullptr) ? initializer->eval() : type->default_value();

    value = compiler->cast_value(value, type->llvm_type());
    if (value == nullptr)
        report_error("Type mismatch between the global variable " + name + " and its initializer");

    auto constant = llvm::dyn_cast<llvm::Constant>(value);
    if (constant == nullptr)
        report_error("The global variable " + name + "'s initializer is not a constant");

    module().getOrInsertGlobal(name, type->llvm_type());
    llvm::GlobalVariable* variable = module().getGlobalVariable(name);
    variable->setInitializer((llvm::Constant*)value);
    variable->setConstant(false);

    symbol_table().insert_global(name, { variable, type });

    return variable;
}

GlobalVariableDefinitionNode::~GlobalVariableDefinitionNode()
{
    delete initializer;
}

}