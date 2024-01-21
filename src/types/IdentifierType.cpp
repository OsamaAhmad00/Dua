#include <types/IdentifierType.hpp>
#include <ModuleCompiler.hpp>

namespace dua
{

const Type* IdentifierType::get_concrete_type() const
{
    if (!is_templated)
    {
        auto type = compiler->get_typing_system().get_type(name);
        if (type != this)
            return type->get_concrete_type();
        return compiler->get_name_resolver().get_class(name);
    }

    auto concrete_types = template_args;
    for (auto& type : concrete_types)
        type = type->get_concrete_type();

    return compiler->get_name_resolver().get_templated_class(name, concrete_types);
}

Value IdentifierType::default_value() const {
    return get_concrete_type()->default_value();
}

llvm::Type *IdentifierType::llvm_type() const {
    return get_concrete_type()->llvm_type();
}

bool IdentifierType::operator==(const Type &other) const {
    return *get_concrete_type() == *other.get_concrete_type();
}

std::string IdentifierType::to_string() const {
     return get_concrete_type()->to_string();
}

std::string IdentifierType::as_key() const
{
    auto result = "Unresolved(" + name;
    if (is_templated) {
        result += '<';
        bool first = true;
        for (auto& arg : template_args) {
            if (!first) result += ".";
            result += arg->as_key();
            first = false;
        }
        result += '>';
    }
    result += ")";
    return result;
}

bool IdentifierType::is_resolvable_now() const
{
    for (auto& arg : template_args)
        if (!arg->is_resolvable_now())
            return false;

    auto full_name = name;
    if (is_templated)
        full_name = compiler->get_name_resolver().get_templated_class_full_name(full_name, template_args);

    return compiler->get_typing_system().identifier_types.contains(full_name);
}

}