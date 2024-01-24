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
            compiler->report_error("The type " + type->to_string() +
                         " a pointer to a non-class type, and can't be used as the target of a dynamic casting operation");
        }
    } else {
        compiler->report_error("The type " + type->to_string() + " is not a pointer type, and can't be used as the target of a dynamic casting operation");
    }

    auto instance_ptr = instance->eval();
    const ClassType* source_class = nullptr;
    if (auto ptr = instance_ptr.type->as<PointerType>(); ptr != nullptr) {
        if (auto cls = ptr->get_element_type()->get_concrete_type()->as<ClassType>(); cls != nullptr) {
            source_class = cls;
        } else {
            compiler->report_error("The type " + instance_ptr.type->to_string() +
                         " is a pointer to a non-class type, and can't be used as the source of a dynamic casting operation");
        }
    } else {
        compiler->report_error("The type " + instance_ptr.type->to_string() +
                     " is not a pointer type, and can't be used as the source of a dynamic casting operation");
    }

    auto target_vtable = name_resolver().get_vtable_instance(target_class->name);

    auto instance_value = compiler->create_value(instance_ptr.get(), source_class);
    auto vtable_ptr_ptr = source_class->get_field(instance_value, ".vtable_ptr");
    auto vtable_type = name_resolver().get_vtable_type(source_class->name)->llvm_type();
    auto init_vtable_ptr = builder().CreateLoad(vtable_type, vtable_ptr_ptr.get(), ".init_vtable");

    auto llvm_ptr_type = llvm::dyn_cast<llvm::PointerType>(pointer_type->llvm_type());
    assert(llvm_ptr_type != nullptr);
    auto casted_ptr = builder().CreatePointerCast(instance_ptr.get(), llvm_ptr_type);
    auto null_ptr = llvm::ConstantPointerNull::get(llvm_ptr_type);

    auto function = module().getFunction(".is_vtable_reachable");
    auto is_reachable = builder().CreateCall(function, { init_vtable_ptr, target_vtable->instance });
    auto as_bool = builder().CreateICmpNE(is_reachable, builder().getInt8(0));

    auto result = builder().CreateSelect(as_bool, casted_ptr, null_ptr);

    return compiler->create_value(result, pointer_type);
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
