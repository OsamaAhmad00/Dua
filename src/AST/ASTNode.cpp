#include <AST/ASTNode.hpp>
#include "types/VoidType.hpp"
#include <utils/ErrorReporting.hpp>

namespace dua
{

llvm::BasicBlock* ASTNode::create_basic_block(const std::string& name, llvm::Function* function) {
    return llvm::BasicBlock::Create(context(), name, function);
}

llvm::AllocaInst* ASTNode::create_local_variable(const std::string& name, const Type* type, Value* init, std::vector<Value> args, bool teleport_value)
{
    llvm::BasicBlock* entry = &current_function()->getEntryBlock();
    temp_builder().SetInsertPoint(entry, entry->begin());
    llvm::AllocaInst* instance = temp_builder().CreateAlloca(type->llvm_type(), 0, name);
    auto value = compiler->create_value(instance, type->get_concrete_type());
    name_resolver().symbol_table.insert(name, value);
    if (init)
    {
        if (!args.empty())
            compiler->report_error("Can't have an both an initializer and an initialization "
                         "list in the definition of a local variable (the variable " + name + ")");
        if (teleport_value) {
            // If the value is teleporting (being moved from one scope to another without
            //  getting destructed), its copy constructor shouldn't be called as well.
            builder().CreateStore(init->get(), instance);
        } else {
            name_resolver().call_copy_constructor(value, *init);
        }
    } else {
        name_resolver().call_constructor(value, std::move(args));
    }
    return instance;
}

NoneValue ASTNode::none_value() { return Value { nullptr, nullptr, nullptr, nullptr }; }

const Type* ASTNode::get_type()
{
    if (type == nullptr)
        return set_type(compiler->create_type<VoidType>());
    return type;
}

const Type* ASTNode::set_type(const Type* type) {
    if (!compiler->stop_caching_types)
        this->type = type;
    return type;
}

}
