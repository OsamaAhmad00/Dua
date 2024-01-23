#include "AST/IndexingNode.hpp"
#include "types/ArrayType.hpp"
#include "AST/lvalue/LoadedLValueNode.hpp"
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"
#include "types/IntegerTypes.hpp"
#include "AST/values/StringValueNode.hpp"

namespace dua
{

const Type* get_element_type(ASTNode* lhs, ASTNode* rhs)
{
    const Type* result = nullptr;
    auto loaded_lvalue = lhs->as<LoadedLValueNode>();
    if (loaded_lvalue != nullptr)
        result = loaded_lvalue->lvalue->get_element_type();
    if (result == nullptr) {
        if (auto i = lhs->as<IndexingNode>(); i != nullptr)
            result = i->get_type();
    }
    if (result == nullptr) {
        if (auto i = lhs->as<StringValueNode>(); i != nullptr)
            result = lhs->get_type();
    }
    if (result == nullptr)
        report_error("There is no indexing operator ([] operator) defined between for the types "
            + lhs->get_type()->to_string() + " and " + rhs->get_type()->to_string());
    return result;
}

Value IndexingNode::eval()
{
    auto lhs_eval = lhs->eval();
    auto rhs_eval = rhs->eval();

    auto postfix_operator_result = name_resolver().call_postfix_operator(lhs_eval, rhs_eval, "Indexing");
    if (!postfix_operator_result.is_null())
        return postfix_operator_result;

    auto index = rhs_eval.cast_as(compiler->create_type<I64Type>(), false);
    if (index.is_null())
        report_error("Can't use a " + rhs_eval.type->to_string() + " as an index");

    auto element_type = get_element_type(lhs, rhs);;
    if (auto ptr = element_type->as<PointerType>(); ptr != nullptr) {
        // Act as if this is an array
        lhs_eval.memory_location = lhs_eval.get();
        element_type = compiler->create_type<ArrayType>(ptr->get_element_type(), LONG_LONG_MAX);
    }

    auto memory_location = builder().CreateGEP(
        element_type->llvm_type(),
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

    auto element_type = get_element_type(lhs, rhs);

    const Type* result = element_type;

    if (auto reference_type = element_type->as<ReferenceType>(); reference_type != nullptr) {
        result = reference_type->get_element_type();
    }

    if (auto pointer_type = element_type->as<PointerType>(); pointer_type != nullptr) {
        result = pointer_type->get_element_type();
    } else if (auto array_type = element_type->as<ArrayType>(); array_type != nullptr) {
        result = array_type->get_element_type();
    }

    if (result == nullptr)
        report_error("Can't use the default index operator with a non-lvalue expression");

    result = compiler->create_type<ReferenceType>(result, false);

    return set_type(result);
}

}
