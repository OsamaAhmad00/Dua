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
    llvm::AllocaInst* instance = temp_builder().CreateAlloca(type->llvm_type(), 0, name);
    auto value = compiler->create_value(instance, type);
    name_resolver().symbol_table.insert(name, value);
    if (init)
    {
        if (!args.empty())
            report_error("Can't have an both an initializer and an initialization "
                         "list in the definition of a local variable (the variable " + name + ")");
        name_resolver().call_copy_constructor(value, *init);
    } else {
        name_resolver().call_constructor(value, std::move(args));
    }
    return instance;
}

NoneValue ASTNode::none_value() { return builder().getInt32(0); }

const Type* ASTNode::get_type()
{
    if (type == nullptr)
        return type = compiler->create_type<VoidType>();
    return type;
}

Value ASTNode::get_eval_value() {
    return compiler->create_value(eval(), get_type());
}

}
