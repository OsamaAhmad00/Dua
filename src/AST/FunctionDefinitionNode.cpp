#include <AST/FunctionDefinitionNode.h>
#include <llvm/IR/Verifier.h>

llvm::Function *FunctionDefinitionNode::eval()
{
    return (body == nullptr) ? declare_function() : define_function();
}


llvm::Function* FunctionDefinitionNode::define_function()
{
    llvm::Function* function = module().getFunction(signature.name);

    if (!function)
        function = declare_function();

    llvm::Function* old_function = current_function();
    llvm::BasicBlock* old_block = builder().GetInsertBlock();
    create_basic_block("entry", function);
    builder().SetInsertPoint(&function->back());
    current_function() = function;

    symbol_table().push_scope();

    int i = 0;
    for (llvm::Argument& arg : function->args()) {
        auto& param = signature.params[i++];
        arg.setName(param.name);
        create_local_variable(param.name, param.type->llvm_type(), &arg);
    }

    body->eval();

    symbol_table().pop_scope();

    builder().SetInsertPoint(old_block);
    current_function() = old_function;

    return function;
}

llvm::Function* FunctionDefinitionNode::declare_function()
{
    llvm::Type* ret = signature.return_type->llvm_type();
    std::vector<llvm::Type*> parameter_types;
    for (auto& param: signature.params)
        parameter_types.push_back(param.type->llvm_type());
    llvm::FunctionType* type = llvm::FunctionType::get(ret, parameter_types, signature.is_var_arg);
    llvm::Function* function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, signature.name, module());
    llvm::verifyFunction(*function);
    return function;
}

FunctionDefinitionNode::~FunctionDefinitionNode()
{
    delete body;
}