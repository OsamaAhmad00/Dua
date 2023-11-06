#include <AST/ASTNode.h>


llvm::BasicBlock* ASTNode::create_basic_block(const std::string& name, llvm::Function* function) {
    return llvm::BasicBlock::Create(context(), name, function);
}

llvm::AllocaInst* ASTNode::create_local_variable(const std::string& name, llvm::Type* type, llvm::Value* init)
{
    llvm::BasicBlock* entry = &current_function()->getEntryBlock();
    temp_builder().SetInsertPoint(entry, entry->begin());
    llvm::AllocaInst* variable = temp_builder().CreateAlloca(type, 0, name);
    if (init) {
        builder().CreateStore(init, variable);
    }
    symbol_table().insert(name, { variable, type });
    return variable;
}

NoneValue ASTNode::none_value() { return builder().getInt32(0); }
