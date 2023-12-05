#pragma once

#include <AST/ASTNode.hpp>
#include <utils/ErrorReporting.hpp>

namespace dua
{

#define SAME_TYPE_BINARY_EXP_NODE(NAME, OP, LABEL)                                    \
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
    static llvm::Value* perform(ModuleCompiler* compiler,                             \
            const Value& lhs, const Value& rhs, const Type* type) {                   \
        auto l = lhs.cast_as(type, false);                                            \
        auto r = rhs.cast_as(type, false);                                            \
        if (l == nullptr || r == nullptr)                                             \
            report_error("Can't perform the operation " #NAME " between the types "   \
                + lhs.type->to_string() + " and " + rhs.type->to_string());           \
        return compiler->get_builder()->OP(l, r, LABEL);                              \
    }                                                                                 \
                                                                                      \
    llvm::Value* eval() override {                                                    \
        auto lhs_value = compiler->create_value(lhs->eval(), lhs->get_type());        \
        auto rhs_value = compiler->create_value(rhs->eval(), rhs->get_type());        \
        return NAME##Node::perform(                                                   \
            compiler,                                                                 \
            lhs_value,                                                                \
            rhs_value,                                                                \
            get_type()                                                                \
        );                                                                            \
    }                                                                                 \
                                                                                      \
    const Type* get_type() override {                                                 \
        if (type != nullptr) return type;                                             \
        auto ltype = lhs->get_type();                                                 \
        auto rtype = rhs->get_type();                                                 \
        return type = ltype->get_winning_type(rtype);                                 \
    }                                                                                 \
};

}
