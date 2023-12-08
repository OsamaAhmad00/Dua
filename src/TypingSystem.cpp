#include <TypingSystem.hpp>
#include <ModuleCompiler.hpp>
#include <utils/ErrorReporting.hpp>
#include <llvm/IR/Type.h>
#include <types/IntegerTypes.hpp>
#include "types/PointerType.hpp"
#include "types/ArrayType.hpp"
#include "types/FloatTypes.hpp"

namespace dua
{

TypingSystem::TypingSystem(ModuleCompiler *compiler) : compiler(compiler) {}

llvm::Value* TypingSystem::cast_value(const Value& value, const Type* type, bool panic_on_failure) const
{
    if (!is_castable(value.type, type)) {
        if (panic_on_failure)
            report_internal_error("Can't cast the type " + value.type->to_string() + " to " + type->to_string());
        return nullptr;
    }

    llvm::Type* source_type = value.ptr->getType();
    llvm::Type* target_type = type->llvm_type();
    llvm::Value* v = value.ptr;

    // If the types are equal, return the value as it is
    if (source_type == target_type) {
        return v;
    }

    llvm::DataLayout dl(&module());
    unsigned int source_width = dl.getTypeAllocSize(source_type);
    unsigned int target_width = dl.getTypeAllocSize(target_type);

    // If the types are both integer types, use the Trunc or ZExt or SExt instructions
    if (source_type->isIntegerTy() && target_type->isIntegerTy())
    {
        if (source_width >= target_width) {
            // This needs to be >=, not just > because of the case of
            //  converting between an i8 and i1, which both give a size
            //  of 1 byte.
            // Truncate the value to fit the smaller type
            return builder().CreateTrunc(v, target_type);
        } else if (source_width < target_width) {
            // Extend the value to fit the larger type
            // A 1 bit number can only store 0 or 1, no negative numbers here.
            // If sign-extended however, the 1 (considered the sign) will propagate,
            // resulting in a binary representation of a bunch of 1s.
            if (source_type == builder().getInt1Ty())
                return builder().CreateZExt(v, target_type);
            return builder().CreateSExt(v, target_type);
        }
    }

    if (source_type->isPointerTy() && target_type->isPointerTy())
        return builder().CreateBitCast(v, target_type);

    if (source_type->isIntegerTy() && target_type->isPointerTy())
        return builder().CreateIntToPtr(v, target_type);

    if (source_type->isFloatingPointTy() && target_type->isFloatingPointTy()) {
        if (source_width > target_width) {
            // Truncate the value to fit the smaller type
            return builder().CreateFPTrunc(v, target_type);
        } else if (source_width < target_width) {
            // Extend the value to fit the larger type
            return builder().CreateFPExt(v, target_type);
        } else {
            // The types have the same bit width, no need to cast
            return v;
        }
    }

    if (source_type->isIntegerTy() && target_type->isFloatingPointTy()) {
        // A 1 bit number can only store 0 or 1, no negative numbers here.
        // If considered to be signed however, the 1 will be considered a sign.
        if (source_type == builder().getInt1Ty())
            return builder().CreateUIToFP(v, target_type);
        return builder().CreateSIToFP(v, target_type);
    }

    if (source_type->isFloatingPointTy() && target_type->isIntegerTy())
        return builder().CreateFPToSI(v, target_type);

    report_internal_error("Casting couldn't be done");
    return nullptr;  // Unreachable
}

const Type* TypingSystem::get_winning_type(const Type* lhs, const Type* rhs, bool panic_on_failure) const
{
    auto l = lhs->llvm_type();
    auto r = rhs->llvm_type();

    if (l == r) {
        return lhs;
    }

    llvm::DataLayout dl(&module());
    unsigned int l_width = dl.getTypeAllocSize(l);
    unsigned int r_width = dl.getTypeAllocSize(r);

    if (l->isIntegerTy() && r->isIntegerTy())
        return (l_width >= r_width) ? lhs : rhs;

    if (l->isPointerTy() && r->isIntegerTy())
        return lhs;

    if (l->isIntegerTy() && r->isPointerTy())
        return rhs;

    if (l->isFloatingPointTy() && r->isFloatingPointTy())
        return (l_width >= r_width) ? lhs : rhs;

    if (l->isIntegerTy() && r->isFloatingPointTy())
        return rhs;

    if (l->isFloatingPointTy() && r->isIntegerTy())
        return lhs;

    if (panic_on_failure)
        report_error("Type mismatch: Can't have the types " + lhs->to_string()
            + " and " + rhs->to_string() + " in an operation");

    return nullptr;
}

llvm::Value* TypingSystem::cast_as_bool(const Value& value, bool panic_on_failure) const {
    auto casted = cast_value(value, create_type<I64Type>(), panic_on_failure);
    return builder().CreateICmpNE(casted, builder().getInt64(0));
}

template <typename T>
static const T* is(const Type* t) { return dynamic_cast<const T*>(t); }

int TypingSystem::similarity_score(const Type *t1, const Type *t2) const
{
    // This function relies on the fact that there is only one instance
    //  of each type. It compares pointers to determine whether the types
    //  are equal or not.

    if (t1 == t2) return 0;

    // For types that must be the same, the above check is enough,
    //  but there are types that are castable to each other, in
    //  which some additional checks are necessary

    if (is<IntegerType>(t1)) {
        if (is<IntegerType>(t2)) return 1;
        if (is<FloatType>(t2))   return 2;
        if (is<PointerType>(t2)) return 3;
        if (is<ArrayType>(t2))   return 4;
        return -1;
    }

    if (is<FloatType>(t1)) {
        if (is<FloatType>(t2))   return 1;
        if (is<IntegerType>(t2)) return 2;
        return -1;
    }

    if (auto ptr1 = is<PointerType>(t1); ptr1 != nullptr) {
        if (auto ptr2 = is<PointerType>(t2); ptr2 != nullptr) {
            auto score = similarity_score(ptr1->get_element_type(), ptr2->get_element_type());
            if (score == -1) return -1;
            return score;
        }
        if (auto arr = is<ArrayType>(t2); arr != nullptr) {
            auto score = similarity_score(ptr1->get_element_type(), arr->get_element_type());
            if (score == -1) return -1;
            return score + 1;
        }
        return -1;
    }

    if (auto arr = is<ArrayType>(t1); arr != nullptr) {
        if (auto ptr = is<PointerType>(t2); ptr != nullptr) {
            auto score = similarity_score(ptr->get_element_type(), arr->get_element_type());
            if (score == -1) return -1;
            return score + 1;
        }
        return -1;
    }

    if (auto f1 = is<FunctionType>(t1); f1 != nullptr) {
        if (auto f2 = is<FunctionType>(t2); f2 != nullptr) {
            int ret_score = similarity_score(f1->return_type, f2->return_type);
            if (ret_score == -1) return -1;
            int param_score = type_list_similarity_score(f1->param_types, f2->param_types);
            if (param_score == -1) return -1;
            return ret_score + param_score;
        }
        return -1;
    }

    return -1;
}

int TypingSystem::type_list_similarity_score(const std::vector<const Type *> &l1,
                                             const std::vector<const Type *> &l2) const
{
    if (l1.size() != l2.size()) return -1;

    int score = 0;
    for (size_t i = 0; i < l1.size(); i++) {
        int type_score = similarity_score(l1[i], l2[i]);
        if (type_score == -1) return -1;
        score += type_score;
    }

    return score;
}

bool TypingSystem::is_castable(const Type *t1, const Type *t2) const {
    return similarity_score(t1, t2) != -1;
}

llvm::IRBuilder<> &TypingSystem::builder() const {
    return compiler->builder;
}

llvm::Module &TypingSystem::module() const {
    return compiler->module;
}

TypingSystem::~TypingSystem()
{
    for (const auto& type : type_cache)
        delete type.second;
}

}