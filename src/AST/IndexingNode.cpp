#include "AST/IndexingNode.hpp"
#include "types/ArrayType.hpp"
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"
#include "types/IntegerTypes.hpp"


namespace dua
{

Value IndexingNode::eval()
{
    auto lhs_eval = lhs->eval();
    auto rhs_eval = rhs->eval();

    auto postfix_operator_result = name_resolver().call_postfix_operator(lhs_eval, rhs_eval, "Indexing");
    if (!postfix_operator_result.is_null())
        return postfix_operator_result;

    auto index = rhs_eval.cast_as(compiler->create_type<I64Type>(), false);
    if (index.is_null())
        compiler->report_error("Can't use a " + rhs_eval.type->to_string() + " as an index");

    // We have to strip the reference away to get the correct (array or pointer) type
    auto type = lhs->get_type()->get_contained_type();
    if (auto ptr = type->as<PointerType>(); ptr != nullptr) {
        // Act as if this is an array
        lhs_eval.memory_location = lhs_eval.get();
        type = compiler->create_type<ArrayType>(ptr->get_element_type(), ULONG_LONG_MAX);
    }

    auto memory_location = builder().CreateGEP(
        type->llvm_type(),
        lhs_eval.memory_location,
        { builder().getInt32(0), rhs_eval.get() }
    );

    return compiler->create_value(get_type(), memory_location);
}

const Type *IndexingNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    auto t = name_resolver().get_postfix_operator_return_type(lhs->get_type(), rhs->get_type(), "Indexing");

    if (t != nullptr)
        return set_type(t);

    auto lhs_type = lhs->get_type();

    const Type* result = lhs_type;

    if (auto reference_type = lhs_type->as<ReferenceType>(); reference_type != nullptr) {
        result = reference_type->get_element_type();
    }

    if (auto pointer_type = lhs_type->as<PointerType>(); pointer_type != nullptr) {
        result = pointer_type->get_element_type();
    } else if (auto array_type = lhs_type->as<ArrayType>(); array_type != nullptr) {
        result = array_type->get_element_type();
    }

    if (result == nullptr)
        compiler->report_error("Can't use the default index operator with a non-lvalue expression");

    result = compiler->create_type<ReferenceType>(result, false);

    return set_type(result);
}

}
