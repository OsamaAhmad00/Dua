#include "AST/lvalue/ArrayIndexingNode.hpp"
#include "types/ArrayType.hpp"

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

const Type *ArrayIndexingNode::get_type() {
    return type = lvalue->get_type();
}

const Type *ArrayIndexingNode::get_element_type() {
    auto element_type = lvalue->get_element_type();
    if (auto arr_type = dynamic_cast<const ArrayType*>(element_type); arr_type != nullptr)
        return arr_type->get_element_type();
    return element_type;
}

}
