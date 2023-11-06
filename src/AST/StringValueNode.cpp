#include <AST/terminals/StringValueNode.h>

int StringValueNode::counter = 0;

llvm::Constant* StringValueNode::eval()
{
    if (is_nullptr) return type.default_value();
    return builder().CreateGlobalStringPtr(value, "StringLiteral" + std::to_string(counter++));
}
