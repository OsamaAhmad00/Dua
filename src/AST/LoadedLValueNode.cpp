#include <AST/lvalue/LoadedLValueNode.h>
#include <types/PointerType.h>

namespace dua
{

llvm::Value* LoadedLValueNode::eval()
{
    return builder().CreateLoad(
        lvalue->get_element_type()->llvm_type(),
        lvalue->eval()
    );
}

TypeBase *LoadedLValueNode::compute_type()
{
    delete type;
    auto result = lvalue->get_cached_type();
    auto ptr = dynamic_cast<PointerType*>(result);
    assert(ptr != nullptr);
    return type = ptr->get_element_type()->clone();
}

}
