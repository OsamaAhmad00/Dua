#include <AST/ASTNode.h>
#include "types/VoidType.h"


llvm::BasicBlock* ASTNode::create_basic_block(const std::string& name, llvm::Function* function) {
    return llvm::BasicBlock::Create(context(), name, function);
}

llvm::AllocaInst* ASTNode::create_local_variable(const std::string& name, TypeBase* type, llvm::Value* init)
{
    llvm::BasicBlock* entry = &current_function()->getEntryBlock();
    temp_builder().SetInsertPoint(entry, entry->begin());
    llvm::AllocaInst* variable = temp_builder().CreateAlloca(type->llvm_type(), 0, name);
    if (init) {
        builder().CreateStore(init, variable);
    }
    symbol_table().insert(name, { variable, type });
    return variable;
}

NoneValue ASTNode::none_value() { return builder().getInt32(0); }

TypeBase *ASTNode::get_cached_type()
{
    if (type == nullptr)
        compute_type();
    return type;
}

TypeBase *ASTNode::compute_type() {
    if (type == nullptr)
        return type = compiler->create_type<VoidType>();
    return type;
}

