#include <AST/variable/GlobalVariableDefinitionNode.h>
#include <AST/LLMVValueNode.h>
#include <utils/ErrorReporting.h>

namespace dua
{

llvm::GlobalVariable* GlobalVariableDefinitionNode::eval()
{
    // ASTNodes evaluation should be idempotent.
    // This condition makes sure this is the case.
    if (result != nullptr)
        return result;

    if (initializer != nullptr && !args.empty())
        report_error("Can't have both an initializer and an initializer list (in " + name + ")");

    module().getOrInsertGlobal(name, type->llvm_type());
    llvm::GlobalVariable* variable = module().getGlobalVariable(name);

    // We're in the global scope now, and the evaluation has to be done inside
    // some basic block. Will move temporarily to the beginning of the main function.
    auto old_position = builder().saveIP();
    builder().SetInsertPoint(&module().getFunction(".dua.init")->getEntryBlock());

    llvm::Constant* constant = nullptr;

    if (initializer != nullptr)
    {
        auto value = initializer->eval();
        delete initializer;

        value = compiler->cast_value(value, type->llvm_type());
        if (value == nullptr)
            report_error("Type mismatch between the global variable " + name + " and its initializer");

        auto casted = llvm::dyn_cast<llvm::Constant>(value);
        // If it's not a constant, then the initialization happens in the
        // .dua.init function. Otherwise, initialized with the constant value.
        if (casted == nullptr) {
            builder().CreateStore(value, variable);
        } else {
            constant = casted;
        }
    }
    else if (!args.empty())
    {
        std::vector<llvm::Value*> llvm_args(args.size());
        for (int i = 0; i < args.size(); i++)
            llvm_args[i] = args[i]->eval();

        compiler->call_constructor({ variable, type }, std::move(llvm_args));
    }

    // Restore the old position back
    builder().restoreIP(old_position);

    variable->setInitializer(constant ? constant : type->default_value());
    variable->setConstant(false);

    symbol_table().insert_global(name, { variable, type });

    return result = variable;
}

}