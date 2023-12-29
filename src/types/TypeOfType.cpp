#include <types/TypeOfType.hpp>
#include <ModuleCompiler.hpp>
#include <AST/lvalue/LoadedLValueNode.hpp>
#include <AST/lvalue/VariableNode.hpp>
#include "types/PointerType.hpp"

namespace dua
{

const Type* TypeOfType::get_concrete_type() const
{
    if (auto l = node->as<LoadedLValueNode>(); l != nullptr) {
        if (auto v = l->lvalue->as<VariableNode>(); v != nullptr) {
            if (v->is_templated)
            {
                // Classes first, then functions
                std::string name;
                name = compiler->get_templated_class_full_name(v->name, v->template_args);
                if (compiler->get_name_resolver().has_class(name))
                    return compiler->get_typing_system().get_type(name)->get_concrete_type();

                name = compiler->get_templated_function_full_name(v->name, v->template_args);
                if (compiler->get_name_resolver().has_function(name)) {
                    auto func_type = compiler->get_name_resolver().get_function_no_overloading(name).type;
                    return compiler->create_type<PointerType>(func_type)->get_concrete_type();
                }
            }
            if (compiler->get_typing_system().identifier_types.contains(v->name)) {
                // This is actually a class type, not a variable expression
                return compiler->get_typing_system().get_type(v->name)->get_concrete_type();
            }
        }
    }
    return node->get_type()->get_concrete_type()->get_concrete_type();
}

Value TypeOfType::default_value() const {
    return get_concrete_type()->default_value();
}

llvm::Type *TypeOfType::llvm_type() const {
    return get_concrete_type()->llvm_type();
}

std::string TypeOfType::to_string() const {
    return get_concrete_type()->to_string();
}

std::string TypeOfType::as_key() const {
    return "TypeOf(" + std::to_string((unsigned long long)node) + ")";
}

}