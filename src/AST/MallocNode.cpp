#include <AST/lvalue/MallocNode.hpp>

namespace dua
{

MallocNode::MallocNode(dua::ModuleCompiler *compiler, dua::Type *type, std::vector<ASTNode*> args)
    : args(std::move(args))
{
    this->compiler = compiler;
    this->type = compiler->create_type<PointerType>(type);
}

llvm::Value *MallocNode::eval()
{
    auto alloc_type = get_element_type()->llvm_type();
    auto size = llvm::DataLayout(&module()).getTypeAllocSize(alloc_type);

    auto instance = compiler->call_function("malloc", { builder().getInt64(size) });

    std::vector<llvm::Value*> llvm_args(args.size());
    for (int i = 0; i < args.size(); i++)
        llvm_args[i] = args[i]->eval();
    compiler->call_constructor({ instance, get_element_type() }, std::move(llvm_args));

    return instance;
}

Type *MallocNode::compute_type() {
    return type;
}

Type *MallocNode::get_element_type() {
     return ((PointerType*)type)->get_element_type();
}

MallocNode::~MallocNode() {
    for (auto& arg : args) delete arg;
}

}