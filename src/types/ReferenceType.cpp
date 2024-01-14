#include <types/ReferenceType.hpp>
#include <utils/ErrorReporting.hpp>
#include <ModuleCompiler.hpp>

namespace dua
{

const Type *ReferenceType::get_concrete_type() const {
    return compiler->create_type<ReferenceType>(element_type->get_concrete_type());
}

dua::Value ReferenceType::default_value() const {
    report_internal_error("Reference types must be initialized");
    return {};
}

bool ReferenceType::operator==(const Type &other) const {
    auto ref = (&other)->as<ReferenceType>();
    return ref && *ref->element_type == *element_type;
}

}
