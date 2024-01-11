#include <AST/class/DynamicCastNode.hpp>
#include "types/PointerType.hpp"

namespace dua
{

Value DynamicCastNode::eval()
{
    auto type = target_type->get_concrete_type();
    auto pointer_type = type->as<PointerType>();
    const ClassType* target_class = nullptr;
    if (pointer_type != nullptr) {
        target_class = pointer_type->get_element_type()->as<ClassType>();
        if (target_class == nullptr) {
            report_error("The type " + type->to_string() +
                         " a pointer to a non-class type, and can't be used as the target of a dynamic casting operation");
        }
    } else {
        report_error("The type " + type->to_string() + " is not a pointer type, and can't be used as the target of a dynamic casting operation");
    }

    auto instance_ptr = instance->eval();
    const ClassType* source_class = nullptr;
    if (auto ptr = instance_ptr.type->as<PointerType>(); ptr != nullptr) {
        if (auto cls = ptr->get_element_type()->get_concrete_type()->as<ClassType>(); cls != nullptr) {
            source_class = cls;
        } else {
            report_error("The type " + instance_ptr.type->to_string() +
                         " is a pointer to a non-class type, and can't be used as the source of a dynamic casting operation");
        }
    } else {
        report_error("The type " + instance_ptr.type->to_string() +
                     " is not a pointer type, and can't be used as the source of a dynamic casting operation");
    }

    auto target_vtable = name_resolver().get_vtable_instance(target_class->name);

    // Loop while the current vtable instance pointer is not the target vtable pointer, or if
    //  you reach the root (a null pointer)
    //  This is equivalent to:
    //      auto ptr = vtable_ptr;
    //      while (ptr && ptr != target_ptr)
    //          ptr = parent_vtable(ptr);
    //      if (ptr == null) return null
    //      return pointer as target_type

    auto instance_value = compiler->create_value(instance_ptr.get(), source_class);
    auto vtable_ptr_ptr = source_class->get_field(instance_value, ".vtable_ptr");
    auto vtable_type = name_resolver().get_vtable_type(source_class->name)->llvm_type();
    auto init_vtable_ptr = builder().CreateLoad(vtable_type, vtable_ptr_ptr.get(), ".init_vtable");

    // Even though vtables of different classes are different in structure, the only part
    //  we're concerned with is the pointer of the parent vtable instance, which is always
    //  in the same position, regardless of the vtable type. For this reason, if we're only
    //  accessing the parent vtable pointer, we can treat any vtable instance as any vtable.
    auto vtable_llvm_type = init_vtable_ptr->getType();

    auto loop_bb = create_basic_block("dynamic_cast_loop", current_function());
    auto proceed_bb = create_basic_block("dynamic_cast_proceed", current_function());
    auto load_parent_bb = create_basic_block("dynamic_cast_load_parent", current_function());
    auto end_bb = create_basic_block("dynamic_cast_end", current_function());

    auto current_vtable = builder().CreateAlloca(vtable_llvm_type, nullptr, "current_vtable");
    builder().CreateStore(init_vtable_ptr, current_vtable);

    builder().CreateBr(loop_bb);
    builder().SetInsertPoint(loop_bb);

    auto current_value = builder().CreateLoad(vtable_llvm_type->getPointerTo(), current_vtable);

    auto is_null = builder().CreateICmpNE(current_value, llvm::Constant::getNullValue(current_value->getType()));
    builder().CreateCondBr(is_null, proceed_bb, end_bb);

    builder().SetInsertPoint(proceed_bb);

    auto is_target = builder().CreateICmpEQ(current_value, target_vtable->instance);
    builder().CreateCondBr(is_target, end_bb, load_parent_bb);

    builder().SetInsertPoint(load_parent_bb);

    // Any vtable is usable here
    auto parent_ptr = target_vtable->get_ith_element(1, vtable_llvm_type, current_value);
    builder().CreateStore(parent_ptr, current_vtable);
    builder().CreateBr(loop_bb);

    builder().SetInsertPoint(end_bb);

    auto llvm_ptr_type = llvm::dyn_cast<llvm::PointerType>(pointer_type->llvm_type());
    assert(llvm_ptr_type != nullptr);
    auto casted_ptr = builder().CreatePointerCast(instance_ptr.get(), llvm_ptr_type);
    auto null_ptr = llvm::ConstantPointerNull::get(llvm_ptr_type);

    auto phi = builder().CreatePHI(llvm_ptr_type, 2, "dynamic_cast_result");
    phi->addIncoming(casted_ptr, proceed_bb);
    phi->addIncoming(null_ptr, loop_bb);

    return compiler->create_value(phi, pointer_type);
}

const Type* DynamicCastNode::get_type()
{
    if (compiler->clear_type_cache)
        type = nullptr;
    if (type == nullptr)
        set_type(target_type->get_concrete_type());
    return type;
}

}
