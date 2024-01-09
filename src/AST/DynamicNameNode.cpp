#include "AST/operators/DynamicNameNode.hpp"
#include "types/PointerType.hpp"
#include "types/IntegerTypes.hpp"

namespace dua
{

Value DynamicNameNode::eval()
{
    auto value = id->eval();
    if (auto type = value.type->as<IntegerType>(); type == nullptr) {
        report_error("The type " + type->to_string() + " is not an integer type, and can't be used in the dynamicname operator");
    }
    return compiler->get_name_resolver().get_class_name(value);
}

const Type* DynamicNameNode::get_type() {
    if (type == nullptr)
        type = compiler->create_type<PointerType>(typing_system().create_type<I8Type>());
    return type;
}

}
