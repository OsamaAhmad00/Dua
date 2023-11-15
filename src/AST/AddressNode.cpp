#include "AST/terminals/lvalue/AddressNode.h"

llvm::Value* AddressNode::eval()
{
    auto ptr = address->eval();

    if (!load_value)
        return ptr;

    // FIXME load different types depending on the context
    module().print(llvm::outs(), nullptr);
    ptr = builder().CreateIntToPtr(ptr, builder().getInt64Ty()->getPointerTo());
    return builder().CreateLoad(builder().getInt64Ty(), ptr);
}

llvm::Type* AddressNode::type() {
    return builder().getPtrTy();
}
