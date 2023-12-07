#include <AST/ASTNode.hpp>
#include "types/VoidType.hpp"

namespace dua
{

llvm::BasicBlock* ASTNode::create_basic_block(const std::string& name, llvm::Function* function) {
    return llvm::BasicBlock::Create(context(), name, function);
}

llvm::AllocaInst* ASTNode::create_local_variable(const std::string& name, const Type* type, Value* init, std::vector<Value> args)
{
    llvm::BasicBlock* entry = &current_function()->getEntryBlock();
    temp_builder().SetInsertPoint(entry, entry->begin());
    size_t sss = args.size();
    llvm::AllocaInst* instance = temp_builder().CreateAlloca(type->llvm_type(), 0, name);
    if (init) {
        auto val = typing_system().cast_value(*init, type);
        builder().CreateStore(val, instance);
    }
    auto value = compiler->create_value(instance, type);
    name_resolver().symbol_table.insert(name, value);
    name_resolver().call_constructor(value, std::move(args));
    return instance;
}

NoneValue ASTNode::none_value() { return builder().getInt32(0); }

const Type* ASTNode::get_type()
{
    if (type == nullptr)
        return type = compiler->create_type<VoidType>();
    return type;
}

}
