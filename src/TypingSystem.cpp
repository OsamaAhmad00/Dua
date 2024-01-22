#include <TypingSystem.hpp>
#include <ModuleCompiler.hpp>
#include <utils/ErrorReporting.hpp>
#include <llvm/IR/Type.h>
#include <types/IntegerTypes.hpp>
#include "types/PointerType.hpp"
#include "types/ArrayType.hpp"
#include "types/FloatTypes.hpp"
#include "types/ReferenceType.hpp"

namespace dua
{

TypingSystem::TypingSystem(ModuleCompiler *compiler) : compiler(compiler) {}

static Value _cast_value(const Value& value, const Type* type, bool panic_on_failure, ModuleCompiler* compiler)
{
    auto& builder = *compiler->get_builder();
    auto& module = *compiler->get_module();

    llvm::Type* source_type = value.get()->getType();
    llvm::Type* target_type = type->llvm_type();

    Value result = value;
    result.type = type;

    // If the types are equal, return the value as it is
    // One reason to compare by llvm types is that reference
    //  types can be allocated or unallocated, both compare
    //  as equal, but have different representations.
    if (source_type == target_type) {
        return result;
    }

    auto source_ref = value.type->as<ReferenceType>();
    auto target_ref = type->as<ReferenceType>();

    // Here, the source and the target can both still be a reference to
    //  the same type, one is allocated and the other is not. They can
    //  be references to different types as well
    // Casting result to a reference is always an allocated reference,
    //  even if the target is unallocated reference, since the creation
    //  of unallocated references has to deal with symbol tables
    if (source_ref)
    {
        if (target_ref) {
            // If the target is reference as well, just return
            //  the pointer. When loaded, will be loaded with
            //  the appropriate type
            if (!source_ref->is_allocated()) {
                assert(result.memory_location != nullptr);
                result.set(result.memory_location);
                result.memory_location = nullptr;
            }
            return result;
        } else {
            // If the target is allocated, load the address
            if (source_ref->is_allocated()) {
                result.memory_location = value.get();
                result.set(nullptr);
            }
            result.type = source_ref->get_element_type();
            return _cast_value(result, type, panic_on_failure, compiler);
        }
    }
    else
    {
        if (target_ref) {
            // If the source is a value type, and the target is a reference
            //  type, just store the address, and when loading, load with the
            //  appropriate type.
            assert(result.memory_location != nullptr);
            result.set(result.memory_location);
            result.memory_location = nullptr;
            return result;
        }
        // else, the below code will handle it
    }

    llvm::DataLayout dl(&module);
    unsigned int source_width = dl.getTypeSizeInBits(source_type);
    unsigned int target_width = dl.getTypeSizeInBits(target_type);

    llvm::Value* v = value.get();

    // If the types are both integer types, use the Trunc or ZExt or SExt instructions
    if (source_type->isIntegerTy() && target_type->isIntegerTy())
    {
        if (source_width >= target_width) {
            // This needs to be >=, not just > because of the case of
            //  converting between an i8 and i1, which both give a size
            //  of 1 byte.
            // Truncate the value to fit the smaller type
            result.set(builder.CreateTrunc(v, target_type));
        } else if (source_width < target_width) {
            // Extend the value to fit the larger type
            // A 1 bit number can only store 0 or 1, no negative numbers here.
            // If sign-extended however, the 1 (considered the sign) will propagate,
            // resulting in a binary representation of a bunch of 1s.
            if (source_type == builder.getInt1Ty())
                result.set(builder.CreateZExt(v, target_type));
            else
                result.set(builder.CreateSExt(v, target_type));
        }
        return result;
    }

    if (source_type->isPointerTy() && target_type->isPointerTy()) {
        result.set(builder.CreateBitCast(v, target_type));
        return result;
    }

    if (source_type->isPointerTy() && target_type->isIntegerTy()) {
        result.set(builder.CreatePtrToInt(v, target_type));
        return result;
    }

    if (source_type->isIntegerTy() && target_type->isPointerTy()) {
        result.set(builder.CreateIntToPtr(v, target_type));
        return result;
    }

    if (source_type->isFloatingPointTy() && target_type->isFloatingPointTy()) {
        if (source_width > target_width) {
            // Truncate the value to fit the smaller type
            result.set(builder.CreateFPTrunc(v, target_type));
            return result;
        } else if (source_width < target_width) {
            // Extend the value to fit the larger type
            result.set(builder.CreateFPExt(v, target_type));
            return result;
        } else {
            // The types have the same bit width, no need to cast
            return result;
        }
    }

    if (source_type->isIntegerTy() && target_type->isFloatingPointTy()) {
        // A 1 bit number can only store 0 or 1, no negative numbers here.
        // If considered to be signed however, the 1 will be considered a sign.
        if (source_type == builder.getInt1Ty())
            result.set(builder.CreateUIToFP(v, target_type));
        else
            result.set(builder.CreateSIToFP(v, target_type));
        return result;
    }

    if (source_type->isFloatingPointTy() && target_type->isIntegerTy()) {
        result.set(builder.CreateFPToSI(v, target_type));
        return result;
    }

    llvm::outs() << "Source type: ";
    source_type->print(llvm::outs());
    llvm::outs() << "\nTarget type: ";
    target_type->print(llvm::outs());
    llvm::outs() << "\n";
    report_internal_error("Casting couldn't be done");
    return result;  // Unreachable
}

Value TypingSystem::cast_value(const dua::Value &value, const Type* target_type, bool panic_on_failure) const
{
    if (!is_castable(value.type, target_type)) {
        if (panic_on_failure)
            report_internal_error("Can't cast the type " + value.type->to_string() + " to " + target_type->to_string());
        return {};
    }

    // Reference cast should be allocated
    if (auto ref = target_type->as<ReferenceType>(); ref != nullptr)
        target_type = ref->get_allocated();

    return _cast_value(value, target_type, panic_on_failure, compiler);
}

const Type* TypingSystem::get_winning_type(const Type* lhs, const Type* rhs, bool panic_on_failure, const std::string& message) const
{
    auto l = lhs->get_contained_type()->llvm_type();
    auto r = rhs->get_contained_type()->llvm_type();

    llvm::DataLayout dl(&module());
    unsigned int l_width = dl.getTypeAllocSize(l);
    unsigned int r_width = dl.getTypeAllocSize(r);

    if (l->isIntegerTy() && r->isIntegerTy())
        return (l_width >= r_width) ? lhs : rhs;

    // TODO should it return the lhs arbitrarily?
    if (l->isPointerTy() && r->isPointerTy())
        return lhs;

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
    {
        if (!message.empty())
            report_error(message);
        else
            report_error("Type mismatch: Can't have the types " + lhs->to_string()
                     + " and " + rhs->to_string() + " in an operation");
    }

    return nullptr;
}

Value TypingSystem::forced_cast_value(const Value& value, const Type *target_type) const {
    auto result = builder().CreateBitOrPointerCast(value.get(), target_type->llvm_type(), "forced_cast");
    return compiler->create_value(result, target_type);
}

Value TypingSystem::cast_as_bool(const Value& value, bool panic_on_failure) const {
    auto casted = cast_value(value, create_type<I64Type>(), panic_on_failure);
    auto result = builder().CreateICmpNE(casted.get(), builder().getInt64(0));
    return compiler->create_value(result, compiler->create_type<I8Type>());
}

template <typename T>
static const T* is(const Type* t) { return dynamic_cast<const T*>(t); }

int TypingSystem::similarity_score(const Type *t1, const Type *t2) const
{
    // Strip any wrappers, including references
    t1 = t1->get_contained_type();
    t2 = t2->get_contained_type();

    // This function relies on the fact that there is only one instance
    //  of each type. It compares pointers to determine whether the types
    //  are equal or not.

    if (t1 == t2) return 0;

    // For types that must be the same, the above check is enough,
    //  but there are types that are castable to each other, in
    //  which some additional checks are necessary

    if (auto c1 = is<ClassType>(t1); c1 != nullptr) {
        if (auto c2 = is<ClassType>(t2); c2 != nullptr) {
            return c1->ancestor_distance(c2);
        }
    }

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
        if (is<IntegerType>(t2)) return 3;  // Just like the score from int to pointer
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
            if (f1->is_var_arg != f2->is_var_arg) return -1;
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
                                             const std::vector<const Type *> &l2, bool can_differ_in_size) const
{
    if (l1.size() != l2.size() && !can_differ_in_size)
        return -1;

    size_t n = std::min(l1.size(), l2.size());

    int score = 0;
    for (size_t i = 0; i < n; i++) {
        int type_score = similarity_score(l1[i], l2[i]);
        if (type_score == -1) return -1;
        score += type_score;
    }

    return score + int(l1.size() - n) + int(l2.size() - n);
}

bool TypingSystem::is_castable(const Type* t1, const Type* t2) const {
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

void TypingSystem::push_scope() {
    identifier_types.push_scope();
}

Scope<const Type*> TypingSystem::pop_scope() {
    return identifier_types.pop_scope();
}

void TypingSystem::insert_type(const std::string &name, const Type *type) {
    identifier_types.insert(name, type->get_concrete_type());
}

void TypingSystem::insert_global_type(const std::string &name, const Type *type) {
    identifier_types.insert_global(name, type->get_concrete_type());
}

const Type* TypingSystem::get_type(const std::string &name) {
    if (identifier_types.contains(name))
        return identifier_types.get(name);
    report_error("The type/alias " + name + " is not defined");
    return nullptr;
}

}