#include <AST/variable/GlobalVariableDefinitionNode.hpp>
#include "AST/values/RawValueNode.hpp"
#include <utils/ErrorReporting.hpp>

namespace dua
{

Value GlobalVariableDefinitionNode::eval()
{
    // ASTNodes evaluation should be idempotent.
    // This condition makes sure this is the case.
    if (!result.is_null())
        return result;

    if (initializer != nullptr && !args.empty())
        compiler->report_error("Can't have both an initializer and an initializer list (in " + name + ")");

    if (is_extern && is_static)
        compiler->report_error("Can't have both the static and the extern options together in the declaration of the global variable " + name);

    module().getOrInsertGlobal(name, type->llvm_type());
    llvm::GlobalVariable* variable = module().getGlobalVariable(name);

    if (is_extern) {
        if (initializer != nullptr)
            compiler->report_error("Extern global variables can't have initializers (in the global variable " + name + " with type " + type->to_string() + ")");
        variable->setLinkage(llvm::GlobalVariable::LinkageTypes::ExternalLinkage);
    } else if (is_static) {
        variable->setLinkage(llvm::GlobalVariable::LinkageTypes::InternalLinkage);
    }

    // We're in the global scope now, and the evaluation has to be done inside
    // some basic block. Will move temporarily to the beginning of the main function.
    auto old_position = builder().saveIP();
    builder().SetInsertPoint(&module().getFunction(".dua.init")->getEntryBlock());

    llvm::Constant* constant = nullptr;

    if (initializer != nullptr)
    {
        auto value = initializer->eval();

        auto llvm_value = typing_system().cast_value(value, type).get();
        if (llvm_value == nullptr)
            compiler->report_error("Type mismatch between the global variable " + name + " and its initializer");

        auto casted = llvm::dyn_cast<llvm::Constant>(llvm_value);
        // If it's not a constant, then the initialization happens in the
        // .dua.init function. Otherwise, initialized with the constant value.
        if (casted == nullptr) {
            builder().CreateStore(llvm_value, variable);
        } else {
            constant = casted;
        }
    }
    else if (!args.empty())
    {
        std::vector<Value> evaluated(args.size());
        for (int i = 0; i < args.size(); i++)
            evaluated[i] = args[i]->eval();

        name_resolver().call_constructor(compiler->create_value(variable, type), std::move(evaluated));
    }

    // Restore the old position back
    builder().restoreIP(old_position);

    if (!is_extern)
    {
        variable->setInitializer(constant ? constant : type->default_value().get_constant());

        auto comdat = module().getOrInsertComdat(name);
        comdat->setSelectionKind(llvm::Comdat::Any);
        variable->setComdat(comdat);
    }

    variable->setConstant(false);

    name_resolver().symbol_table.insert_global(name, compiler->create_value(variable, type));

    return result = compiler->create_value(variable, get_type());
}

}