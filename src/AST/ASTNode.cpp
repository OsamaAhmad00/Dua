#include <AST/ASTNode.hpp>
#include "types/ReferenceType.hpp"
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
    auto value = compiler->create_value(instance, type->get_concrete_type());

    if (!name.empty())
        name_resolver().symbol_table.insert(name, value);

    if (init)
    {
        if (!args.empty()) {
            compiler->report_error("Can't have an both an initializer and an initialization "
                                   "list in the definition of a local variable (the variable " + name + ")");
        }
        name_resolver().copy_construct(value, *init);
    } else {
        name_resolver().call_constructor(value, std::move(args));
    }

    return instance;
}

std::vector<Value> ASTNode::eval_args(const std::vector<ASTNode*>& args)
{
    auto n = args.size();
    std::vector<Value> evaluated(n);
    for (size_t i = 0; i < n; i++)
    {
        // nullptr args are placeholders that
        //  will be substituted later
        if (args[i] != nullptr)
            evaluated[i] = args[i]->eval();
    }

    return evaluated;
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
