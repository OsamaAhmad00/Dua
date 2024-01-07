#include <AST/operators/ClassIDNode.hpp>
#include <types/IntegerTypes.hpp>
#include "AST/lvalue/LoadedLValueNode.hpp"
#include "AST/lvalue/VariableNode.hpp"
#include "types/PointerType.hpp"
#include "types/ReferenceType.hpp"


namespace dua
{

Value ClassIDNode::eval()
{
    assert(instance == nullptr || target_type == nullptr);

    auto type = target_type;
    llvm::Value* instance_ptr = nullptr;

    bool evaluate = instance != nullptr;

    if (evaluate) {
        // If it's a loaded lvalue, we check if it's actually a
        //  variable reference or a type name that the parser
        //  confused it with a variable reference
        if (auto l = instance->as<LoadedLValueNode>(); l != nullptr) {
            if (auto v = l->lvalue->as<VariableNode>(); v != nullptr) {
                auto variable_name = v->get_name();
                if (v->is_templated) {
                    std::string name = compiler->get_name_resolver().get_templated_function_full_name(variable_name,
                                                                                                      v->template_args);
                    if (compiler->get_name_resolver().has_function(name)) {
                        auto func_type = compiler->get_name_resolver().get_function_no_overloading(name).type;
                        type = compiler->create_type<PointerType>(func_type)->get_concrete_type();
                    }

                    // If this is not a function, try as a templated class, and instantiate it if not instantiated yet.
                    name = compiler->get_name_resolver().get_templated_class_full_name(variable_name, v->template_args);
                    if (!compiler->get_name_resolver().has_class(name)) {
                        compiler->get_name_resolver().register_templated_class(variable_name, v->template_args);
                        // compiler->get_name_resolver().define_templated_class(variable_name, v->template_args);
                    }
                    type = compiler->get_typing_system().get_type(name)->get_concrete_type();

                    evaluate = false;
                }
                if (compiler->get_typing_system().identifier_types.contains(variable_name)) {
                    // This is actually a class type, or a type alias, not a variable expression
                    type = compiler->get_typing_system().get_type(variable_name)->get_concrete_type();
                    evaluate = false;
                }
            }
        }
    }

    if (evaluate) {
        auto eval = instance->eval();
        type = eval.type;
        instance_ptr = eval.memory_location;
    }

    auto concrete_type = type->get_concrete_type();
    auto class_type = concrete_type->as<ClassType>();

    if (class_type == nullptr)
        report_error("The type " + concrete_type->to_string() + " is not a class type, and doesn't have a class ID");

    auto vtable = name_resolver().get_vtable_instance(class_type->name);
    llvm::Value* vtable_ptr = nullptr;

    if (instance_ptr != nullptr) {
        auto value = compiler->create_value(instance_ptr, class_type);
        auto vtable_ptr_ptr = class_type->get_field(value, ".vtable_ptr");
        auto vtable_type = name_resolver().get_vtable_type(class_type->name)->llvm_type();
        vtable_ptr = builder().CreateLoad(vtable_type, vtable_ptr_ptr.get(), ".vtable");
    }

    auto i64 = compiler->create_type<I64Type>();
    auto ptr = vtable->get_ith_element(0, i64->llvm_type(), vtable_ptr);

    return compiler->create_value(ptr, i64);
}

}

const dua::Type *dua::ClassIDNode::get_type() {
    return set_type(compiler->create_type<I64Type>());
}
