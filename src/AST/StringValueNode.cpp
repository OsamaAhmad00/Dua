#include <AST/values/StringValueNode.hpp>
#include "types/PointerType.hpp"
#include "types/IntegerTypes.hpp"

namespace dua
{

int StringValueNode::counter = 0;

Value StringValueNode::eval()
{
    auto name = ".StringLiteral" + std::to_string(counter++);

    if (!compiler->include_libdua)
        return compiler->create_string(name, value);

    // Constructor args
    // 1 - buffer
    Value buffer = compiler->create_string(name, value);
    // 2 - size (+ 1 for the null terminator)
    Value size = compiler->create_value(builder().getInt64(value.size() + 1), compiler->create_type<I64Type>());
    // 3, 4 - true
    Value true_value = compiler->create_value(builder().getInt8(1), compiler->create_type<I8Type>());

    auto type = get_type();

    auto result = compiler->create_local_variable(name, type, nullptr, { buffer, size, true_value, true_value });

    return compiler->create_value(type, result);
}

const Type *StringValueNode::get_type()
{
    if (type == nullptr) {
        if (compiler->include_libdua)
            return set_type(name_resolver().get_class("String"));
        return set_type(compiler->create_type<PointerType>(compiler->create_type<I8Type>()));
    }
    return type;
}

}
