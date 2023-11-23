#include <AST/function/FunctionDefinitionNode.h>
#include <types/VoidType.h>

namespace dua
{

llvm::Function *FunctionDefinitionNode::eval()
{
    // The declaration logic is moved to the parser,
    //  so that all functions are visible everywhere
    //  across the module, regardless of the order
    //  of declaration/definition.
    if (body == nullptr)
        return module().getFunction(name);
    return define_function();
}

llvm::Function* FunctionDefinitionNode::define_function()
{
    auto& signature = compiler->get_function(name);
    llvm::Function* function = module().getFunction(name);

    if (!function)
        report_internal_error("Use of an undeclared function");

    llvm::Function* old_function = current_function();
    llvm::BasicBlock* old_block = builder().GetInsertBlock();
    llvm::BasicBlock* current_block = create_basic_block("entry", function);
    builder().SetInsertPoint(current_block);
    current_function() = function;

    symbol_table().push_scope();

    int i = 0;
    for (llvm::Argument& arg : function->args()) {
        auto& param = signature.params[i++];
        arg.setName(param.name);
        create_local_variable(param.name, param.type, &arg);
    }

    body->eval();

    if (signature.return_type->llvm_type() == builder().getVoidTy())
        builder().CreateRetVoid();
    else {
        // TODO do a more sophisticated analysis
        auto terminator = builder().GetInsertBlock()->getTerminator();
        if (!terminator || llvm::dyn_cast<llvm::ReturnInst>(terminator) == nullptr) {
            builder().CreateRet(signature.return_type->default_value());
        }
    }

    symbol_table().pop_scope();

    builder().SetInsertPoint(old_block);
    current_function() = old_function;

    return function;
}

FunctionDefinitionNode::~FunctionDefinitionNode()
{
    delete body;
}

}
