#include <AST/ASTNode.h>
#include "types/VoidType.h"

namespace dua
{

llvm::BasicBlock* ASTNode::create_basic_block(const std::string& name, llvm::Function* function) {
    return llvm::BasicBlock::Create(context(), name, function);
}

llvm::AllocaInst* ASTNode::create_local_variable(const std::string& name, TypeBase* type, llvm::Value* init)
{
    llvm::BasicBlock* entry = &current_function()->getEntryBlock();
    temp_builder().SetInsertPoint(entry, entry->begin());
    llvm::AllocaInst* instance = temp_builder().CreateAlloca(type->llvm_type(), 0, name);
    if (init) {
        init = compiler->cast_value(init, type->llvm_type());
        builder().CreateStore(init, instance);
    }
    auto variable = ModuleCompiler::Variable { instance, type };
    symbol_table().insert(name, variable);
    compiler->call_method_if_exists(variable, "constructor");
    return instance;
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

}
