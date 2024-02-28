#include <TypingSystem.hpp>
#include <ModuleCompiler.hpp>
#include <llvm/IR/Type.h>
#include <types/IntegerTypes.hpp>
#include "types/PointerType.hpp"
#include "types/ArrayType.hpp"
#include "types/FloatTypes.hpp"
#include "types/ReferenceType.hpp"
#include "types/NullType.hpp"

namespace dua
{

TypingSystem::TypingSystem(ModuleCompiler *compiler) : compiler(compiler), identifier_types(compiler) {}

static Value _cast_value(const Value& value, const Type* type, bool panic_on_failure, ModuleCompiler* compiler)
{
    auto& builder = *compiler->get_builder();
    auto& module = *compiler->get_module();

    llvm::Type* source_type = value.get()->getType();
    llvm::Type* target_type = type->llvm_type();

    // Result is copied to reserve other properties of the value (such as is_teleporting)
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

    if (auto arr = value.type->as<ArrayType>(); arr != nullptr)
    {
        if (*arr == *type)
            return result;

        if (value.memory_location == nullptr) {
            compiler->report_internal_error("Can't cast the array with type " + arr->to_string()
                + " with no memory location to a type other than its array type (to " + type->to_string() + ")");
        }

        source_type = arr->get_element_type()->llvm_type()->getPointerTo();
        v = builder.CreateBitOrPointerCast(value.memory_location, source_type);
    }

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

    // Convert from String to i8*
    if (type == compiler->create_type<PointerType>(compiler->create_type<I8Type>())) {
        if (auto str = value.type->get_contained_type()->as<ClassType>(); str != nullptr && str->name == "String") {
            auto instance = value;
            instance.set(instance.memory_location);
            auto buffer = str->get_field(instance, "buffer");
            buffer.memory_location = buffer.get();
            buffer.set(nullptr);
            return buffer;
        }
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

    llvm::errs() << "\nSource type: ";
    source_type->print(llvm::errs());
    llvm::errs() << "\nTarget type: ";
    target_type->print(llvm::errs());
    llvm::errs() << "\n";
    compiler->report_internal_error("Casting couldn't be done");
    return result;  // Unreachable
}

Value TypingSystem::cast_value(const dua::Value &value, const Type* target_type, bool panic_on_failure) const
{
    if (!is_castable(value.type, target_type)) {
        if (panic_on_failure)
            compiler->report_error("Can't cast the type " + value.type->to_string() + " to " + target_type->to_string());
        return {};
    }

    const ReferenceType* unallocated = nullptr;
    // Reference cast should be allocated
    if (auto ref = target_type->as<ReferenceType>(); ref != nullptr) {
        if (!ref->is_allocated()) {
            unallocated = ref;
            target_type = ref->get_allocated();
        }
    }

    auto result = _cast_value(value, target_type, panic_on_failure, compiler);

    if (unallocated != nullptr) {
        result.type = unallocated;
        result.memory_location = result.get();
        result.set(nullptr);
    }

    return result;
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
            compiler->report_error(message);
        else
            compiler->report_error("Type mismatch: Can't have the types " + lhs->to_string()
                     + " and " + rhs->to_string() + " in an operation");
    }

    return nullptr;
}

Value TypingSystem::forced_cast_value(const Value& value, const Type *target_type) const
{
    // TODO check if the source type is not castable under any condition (such as casting a pointer to a function type)
    // report_error("The type " + value.type->to_string() + " can't be casted in any way to the type " + target_type->to_string());
    auto casted = builder().CreateBitOrPointerCast(value.get(), target_type->llvm_type(), "forced_cast");
    auto result = value;
    result.set(casted);
    result.type = target_type;
    return result;
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
    // This function relies on the fact that there is only one instance
    //  of each type. It compares pointers to determine whether the types
    //  are equal or not.

    // This function is not bidirectional. For example, an i32& can be
    //  bound to an i64 or an i16. But an i32 can't be bound to neither
    //  an i64& nor an i16&. This is because a reference to a larger size
    //  would result in writes exceeding the amount of memory of the original
    //  variable. Same for binding to smaller types, which will write only
    //  to a portion of the variable memory, and leave the rest as it is,
    //  resulting in a garbage value.

    // Yes, this is correct. Only strip the reference
    //  from the source type, but not the target type.
    t1 = t1->get_concrete_type()->get_contained_type();
    t2 = t2->get_concrete_type();

    // Reference and non-reference types with the same element type are interchangeable
    if (t1 == t2 || t1 == t2->get_contained_type()) return 0;

    // This is needed for the case of two equivalent types with two different class types,
    //  such as an IdentifierType and a ClassType, both referring to the same type.
    if (*t1 == *t2 || *t1 == *t2->get_contained_type()) return 0;

    // For types that must be the same, the above check is enough,
    //  but there are types that are castable to each other, in
    //  which some additional checks are necessary

    if (auto r2 = is<ReferenceType>(t2); r2 != nullptr) {
        if (auto c2 = is<ClassType>(r2->get_element_type()); c2 != nullptr) {
            if (auto c1 = is<ClassType>(t1); c1 != nullptr) {
                return c1->ancestor_distance(c2);
            }
        }
        return -1;
    }

    if (auto c1 = is<ClassType>(t1); c1 != nullptr)
    {
        // Convert from String to i8*
        if (c1->name == "String") {
            if (t2 == compiler->create_type<PointerType>(compiler->create_type<I8Type>())) {
                // 1 for i8* to i8*
                // 2 for ancestors
                return 3;
            }
        }
        return -1;
    }

    if (auto i2 = is<IntegerType>(t2))
    {
        if (auto i1 = is<IntegerType>(t1)) {
            // i2 is the target. If the target is bigger, it's ok
            //  because the target can hold the smaller size. But
            //  if the target is smaller, the target might not be
            //  able to hold the whole number. Here, if the types
            //  are not the same, a bigger type is favorable. A float
            //  type is more favorable than a smaller integer type.
            int o1 = i1->size_order();
            int o2 = i2->size_order();
            if (o2 > o1) {
                return o2 - o1;  // Max diff = 4 - 1 = 3
            } else {
                return 4 + o1 - o2; // Max = 4 + 4 - 1 = 7
            }
        }
        if (is<FloatType>(t1))   return 4;
        return -1;
    }

    if (is<FloatType>(t2))
    {
        if (is<FloatType>(t1))   return 1;
        if (is<IntegerType>(t1)) return 2;
        return -1;
    }

    if (auto ptr2 = is<PointerType>(t2); ptr2 != nullptr)
    {
        if (auto ptr1 = is<PointerType>(t1); ptr1 != nullptr)
        {
            auto e1 = ptr1->get_element_type();
            auto e2 = ptr2->get_element_type();

            auto null = compiler->create_type<NullType>();
            if (e1 == null || e2 == null) {
                // A null pointer can be of any type
                return 1;
            }

            auto c1 = e1->as<ClassType>();
            auto c2 = e2->as<ClassType>();
            if (c1 != nullptr && c2 != nullptr)
                return c1->ancestor_distance(c2);

            return -1;
        }

        if (auto arr = is<ArrayType>(t1); arr != nullptr)
        {
            if (arr->get_element_type() == ptr2->get_element_type()) return 1;
            return -1;
        }

        return -1;
    }

    if (auto arr = is<ArrayType>(t2); arr != nullptr)
    {
        if (auto ptr = is<PointerType>(t1); ptr != nullptr)
            if (arr->get_element_type() == ptr->get_element_type())
                return 1;
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
    compiler->report_error("The type/alias " + name + " is not defined");
    return nullptr;
}

}