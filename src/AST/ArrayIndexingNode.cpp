#include "AST/lvalue/ArrayIndexingNode.hpp"
#include "types/ArrayType.hpp"

namespace dua
{

Value ArrayIndexingNode::eval()
{
    auto eval = lvalue->eval();
    auto value = builder().CreateGEP(
        lvalue->get_element_type()->llvm_type(),
        eval.get(),
        { builder().getInt32(0), index->eval().get() }
    );
    return compiler->create_value(value, get_type());
}

const Type *ArrayIndexingNode::get_type() {
    return set_type(lvalue->get_element_type());
}

const Type *ArrayIndexingNode::get_element_type() {
    auto element_type = lvalue->get_element_type();
    if (auto arr_type = dynamic_cast<const ArrayType*>(element_type); arr_type != nullptr)
        return arr_type->get_element_type();
    return element_type;
}

}
