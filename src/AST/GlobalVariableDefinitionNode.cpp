#include <AST/variable/GlobalVariableDefinitionNode.hpp>
#include <types/ReferenceType.hpp>

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

    if (module().getNamedGlobal(name) != nullptr)
        compiler->report_error("The global variable " + name + " is already defined");

    module().getOrInsertGlobal(name, type->llvm_type());
    llvm::GlobalVariable* variable = module().getNamedGlobal(name);

    if (is_extern) {
        if (initializer != nullptr)
            compiler->report_error("Extern global variables can't have initializers (in the global variable " + name + " with type " + type->to_string() + ")");
        variable->setLinkage(llvm::GlobalVariable::LinkageTypes::ExternalLinkage);
    } else if (is_static) {
        variable->setLinkage(llvm::GlobalVariable::LinkageTypes::InternalLinkage);
    }

    // We're in the global scope now, and the evaluation has to be done inside
    //  some basic block. Will move temporarily to the beginning of the .dua.init
    //  function for calling the constructor, and .dua.cleanup function for calling
    //  the destructor.

    auto old_position = builder().saveIP();
    auto old_function = current_function();

    current_function() = compiler->get_dua_init_function();
    builder().SetInsertPoint(&current_function()->getEntryBlock());

    if (!is_extern)
    {
        variable->setInitializer(type->default_value().get_constant());

        std::vector<Value> evaluated_args(args.size());
        for (int i = 0; i < args.size(); i++)
            evaluated_args[i] = args[i]->eval();

        Value init_value;
        if (initializer != nullptr) init_value = initializer->eval();
        auto init = initializer ? &init_value : nullptr;

        auto initializer = compiler->create_local_variable(name + "_initializer", type, init, std::move(evaluated_args), false);
        auto value = compiler->create_value(type, initializer);
        value.is_teleporting = true;
        auto instance = compiler->create_value(variable, type);
        name_resolver().copy_construct(instance, value);

        auto comdat = module().getOrInsertComdat(name);
        comdat->setSelectionKind(llvm::Comdat::Any);
        variable->setComdat(comdat);
    }

    // Restore the old position back
    builder().restoreIP(old_position);
    current_function() = old_function;

    variable->setConstant(false);

    name_resolver().symbol_table.insert_global(name, compiler->create_value(variable, type));

    return result = compiler->create_value(variable, get_type());
}


}