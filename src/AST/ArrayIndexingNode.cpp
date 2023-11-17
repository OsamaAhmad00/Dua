#include "AST/lvalue/ArrayIndexingNode.h"
#include "types/ArrayType.h"

namespace dua
{

llvm::Value *ArrayIndexingNode::eval()
{
    auto ptr = lvalue->eval();
    return builder().CreateGEP(
        lvalue->get_element_type()->llvm_type(),
        ptr,
        { builder().getInt32(0), index->eval() }
    );
}

TypeBase *ArrayIndexingNode::compute_type()
{
    delete type;
    return type = lvalue->get_cached_type()->clone();
}

TypeBase *ArrayIndexingNode::get_element_type() {
    auto element_type = lvalue->get_element_type();
    if (auto arr_type = dynamic_cast<ArrayType*>(element_type); arr_type != nullptr)
        return arr_type->get_element_type();
    return element_type;
}

ArrayIndexingNode::~ArrayIndexingNode() {
    delete lvalue;
}

}
