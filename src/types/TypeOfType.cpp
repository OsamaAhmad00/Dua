#include <types/TypeOfType.hpp>
#include <ModuleCompiler.hpp>
#include <AST/lvalue/LoadedLValueNode.hpp>
#include <AST/lvalue/VariableNode.hpp>

namespace dua
{

const Type* TypeOfType::get_concrete_type() const
{
    // TODO improve this so that it doesn't
    if (auto l = node->as<LoadedLValueNode>(); l != nullptr) {
        if (auto v = l->lvalue->as<VariableNode>(); v != nullptr) {
            if (compiler->get_typing_system().identifier_types.contains(v->name)) {
                // This is actually a class type, not a variable expression
                return compiler->get_typing_system().get_type(v->name);
            }
        }
    }
    return node->get_type();
}

Value TypeOfType::default_value() const {
    return get_concrete_type()->default_value();
}

llvm::Type *TypeOfType::llvm_type() const {
    return get_concrete_type()->llvm_type();
}

bool TypeOfType::operator==(const Type &other) {
    return *get_concrete_type() == other;
}

std::string TypeOfType::to_string() const {
    return get_concrete_type()->to_string();
}

std::string TypeOfType::as_key() const {
    return "TypeOf(" + std::to_string((unsigned long long)node) + ")";
}

}