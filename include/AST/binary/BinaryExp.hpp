#pragma once

#include <AST/ASTNode.hpp>
#include <types/FloatTypes.hpp>

namespace dua
{

#define SAME_TYPE_BINARY_EXP_NODE(NAME, INT_OP, FLOAT_OP, LABEL, NO_FLOAT)            \
class NAME##Node : public ASTNode                                                     \
{                                                                                     \
    ASTNode* lhs;                                                                     \
    ASTNode* rhs;                                                                     \
                                                                                      \
public:                                                                               \
                                                                                      \
    NAME##Node(ModuleCompiler* compiler, ASTNode* lhs, ASTNode* rhs)                  \
        : lhs(lhs), rhs(rhs) { this->compiler = compiler; }                           \
                                                                                      \
    static Value perform(ModuleCompiler* compiler,                                    \
            const Value& lhs, const Value& rhs, const Type* type)                     \
    {                                                                                 \
        /* Try calling infix operator first */                                        \
        auto infix_call = compiler->get_name_resolver()                               \
                .call_infix_operator(lhs, rhs, #NAME);                                \
        if (!infix_call.is_null())                                                    \
            return infix_call;                                                        \
                                                                                      \
        auto l = lhs.cast_as(type, false);                                            \
        auto r = rhs.cast_as(type, false);                                            \
        if (l.is_null() || r.is_null())                                               \
            compiler->report_error("Can't perform the operation "                     \
                #NAME " between the types "                                           \
                + lhs.type->to_string() + " and " + rhs.type->to_string());           \
        llvm::Value* ptr;                                                             \
        if (type->as<FloatType>()) {                                                  \
            if (NO_FLOAT)                                                             \
                compiler->report_error("The operation " #NAME                         \
                    " is applicable only on integer types");                          \
            ptr = compiler->get_builder()->FLOAT_OP(l.get(), r.get(), LABEL);         \
        } else {                                                                      \
            ptr = compiler->get_builder()->INT_OP(l.get(), r.get(), LABEL);           \
        }                                                                             \
                                                                                      \
        /* This is necessary for making sure that the type returned is actually */    \
        /*  the desired type. Even though we're casting to the same dua::Type   */    \
        /*  type, which the typing system will allow, the underlying llvm::Type */    \
        /*  might be different, and we have to make sure that it's the correct  */    \
        /*  type too.                                                           */    \
        auto value = compiler->create_value(ptr, type);                               \
        return value.cast_as(type);                                                   \
    }                                                                                 \
                                                                                      \
    Value eval() override                                                             \
    {                                                                                 \
        auto lhs_value = lhs->eval();                                                 \
        auto rhs_value = rhs->eval();                                                 \
                                                                                      \
        return NAME##Node::perform(                                                   \
            compiler,                                                                 \
            lhs_value,                                                                \
            rhs_value,                                                                \
            get_type()                                                                \
        );                                                                            \
    }                                                                                 \
                                                                                      \
    const Type* get_type() override                                                   \
    {                                                                                 \
        if (compiler->clear_type_cache) type = nullptr;                               \
                                                                                      \
        if (type != nullptr) return type;                                             \
                                                                                      \
        auto ltype = lhs->get_type()->get_contained_type();                           \
        auto rtype = rhs->get_type()->get_contained_type();                           \
                                                                                      \
        auto infix_type = name_resolver()                                             \
            .get_infix_operator_return_type(ltype, rtype, #NAME);                     \
                                                                                      \
        if (infix_type != nullptr) return set_type(infix_type);                       \
                                                                                      \
        return set_type(ltype->get_winning_type(rtype, true,                          \
            "There is no " #NAME " operator defined for the types " +                 \
            ltype->to_string() + " and " + rtype->to_string()));                      \
    }                                                                                 \
                                                                                      \
    static std::string operation_name() { return #NAME; }                             \
};

}
