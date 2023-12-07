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

    // TODO is setting the size to 1 is the best solution for non-sized struct types?
    size_t size = alloc_type->isSized() ? llvm::DataLayout(&module()).getTypeAllocSize(alloc_type) : 1;


    // Even though the returned type is I64*, we'll pretend it's a pointer type to the target type.
    //  LLVM typing system will allow substitution of any pointer in place of the other, but our
    //  typing system won't allow this, so this is a quick hack to avoid it complaining.
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