#include <AST/variable/GlobalVariableDefinitionNode.h>
#include <AST/AssignmentExpressionNode.h>
#include <AST/lvalue/VariableNode.h>
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

    llvm::Constant* constant;

    if (initializer != nullptr)
    {
        // We're in the global scope now, and the evaluation has to be done inside
        // some basic block. Will move temporarily to the beginning of the main function.
        auto old_position = builder().saveIP();
        builder().SetInsertPoint(&module().getFunction(".dua.init")->getEntryBlock());

        auto value = initializer->eval();

        value = compiler->cast_value(value, type->llvm_type());
        if (value == nullptr)
            report_error("Type mismatch between the global variable " + name + " and its initializer");

        constant = llvm::dyn_cast<llvm::Constant>(value);
        if (constant == nullptr)
        {
            // Moving ownership of the initializer to the assignment expression
            auto assignment = compiler->create_node<AssignmentExpressionNode>(
                    compiler->create_node<VariableNode>(name),
                    compiler->create_node<LLVMValueNode>(value, initializer->get_cached_type()->clone())
            );
            initializer = nullptr;

            compiler->push_deferred_node(assignment);

            constant = type->default_value();
        }

        // Restore the old position back
        builder().restoreIP(old_position);
    } else {
        constant = type->default_value();
    }

    module().getOrInsertGlobal(name, type->llvm_type());
    llvm::GlobalVariable* variable = module().getGlobalVariable(name);
    variable->setInitializer(constant);
    variable->setConstant(false);

    symbol_table().insert_global(name, { variable, type });

    return result = variable;
}

}