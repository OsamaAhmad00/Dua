#include <types/ReferenceType.hpp>
#include <utils/ErrorReporting.hpp>
#include <ModuleCompiler.hpp>

namespace dua
{

const Type *ReferenceType::get_concrete_type() const {
    return compiler->create_type<ReferenceType>(element_type->get_concrete_type(), is_allocated());
}

dua::Value ReferenceType::default_value() const {
    report_internal_error("Reference types must be initialized");
    return {};
}

bool ReferenceType::operator==(const Type &other) const {
    auto ref = (&other)->as<ReferenceType>();
    // Being allocated or not doesn't affect comparisons
    return ref && *ref->element_type == *element_type;
}

llvm::Type *ReferenceType::llvm_type() const
{
    auto type = element_type->llvm_type();
    if (is_allocated())
        type = type->getPointerTo();
    return type;
}

std::string ReferenceType::as_key() const {
    return element_type->as_key() + (is_allocated() ? "a" : "" ) + "ref";
}

std::string ReferenceType::to_string() const {
    return element_type->to_string() + "&";
}

const Type *ReferenceType::get_element_type() const {
     return element_type;
}

const ReferenceType *ReferenceType::get_allocated() const {
    if (is_allocated())
        return this;
    return compiler->create_type<ReferenceType>(element_type, true);
}

const ReferenceType *ReferenceType::get_unallocated() const {
    if (!is_allocated())
        return this;
    return compiler->create_type<ReferenceType>(element_type, false);
}

bool ReferenceType::is_allocated() const {
    return _is_allocated;
}

}
