#include <AST/function/FunctionDefinitionNode.h>
#include <llvm/IR/Verifier.h>
#include <types/VoidType.h>

llvm::Function *FunctionDefinitionNode::eval()
{
    return (body == nullptr) ? declare_function() : define_function();
}

llvm::Function* FunctionDefinitionNode::define_function()
{
    auto& signature = compiler->get_function(name);
    llvm::Function* function = module().getFunction(name);

    if (!function)
        function = declare_function();

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

    symbol_table().pop_scope();

    builder().SetInsertPoint(old_block);
    current_function() = old_function;

    return function;
}

llvm::Function* FunctionDefinitionNode::declare_function()
{
    auto& signature = compiler->get_function(name);
    llvm::Type* ret = signature.return_type->llvm_type();
    std::vector<llvm::Type*> parameter_types;
    for (auto& param: signature.params)
        parameter_types.push_back(param.type->llvm_type());
    llvm::FunctionType* type = llvm::FunctionType::get(ret, parameter_types, signature.is_var_arg);
    llvm::Function* function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, module());
    llvm::verifyFunction(*function);

    return function;
}

FunctionDefinitionNode::~FunctionDefinitionNode()
{
    delete body;
}