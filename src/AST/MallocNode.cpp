#include <AST/lvalue/MallocNode.hpp>
#include "types/IntegerTypes.hpp"

namespace dua
{

MallocNode::MallocNode(dua::ModuleCompiler *compiler, const Type *type, std::vector<ASTNode*> args)
    : args(std::move(args))
{
    this->compiler = compiler;
    this->type = compiler->create_type<PointerType>(type);
}

llvm::Value *MallocNode::eval()
{
    auto alloc_type = get_element_type()->llvm_type();
    auto size = llvm::DataLayout(&module()).getTypeAllocSize(alloc_type);

    auto instance = name_resolver().call_function(
        "malloc",
         { compiler->create_value(builder().getInt64(size), compiler->create_type<I64Type>()) }
    );

    std::vector<Value> evaluated(args.size());
    for (int i = 0; i < args.size(); i++)
        evaluated[i] = compiler->create_value(args[i]->eval(), args[i]->get_type());
    name_resolver().call_constructor(compiler->create_value(instance, get_element_type()), std::move(evaluated));

    return instance;
}

const Type *MallocNode::get_type() {
    return type;
}

const Type *MallocNode::get_element_type() {
     return ((PointerType*)type)->get_element_type();
}

}