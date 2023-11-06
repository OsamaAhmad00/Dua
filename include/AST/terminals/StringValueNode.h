#include <AST/terminals/ValueNode.h>

class StringValueNode : public ValueNode {

    bool is_nullptr;
    std::string value;

    static int counter;

public:

    StringValueNode() : is_nullptr(true) {}
    StringValueNode(std::string value) : is_nullptr(false), value(std::move(value)) {}
    llvm::Constant* eval() override;
    llvm::Constant *default_value() override { return llvm::Constant::getNullValue(llvm_type()); }
    llvm::Type *llvm_type() override { return builder().getInt8PtrTy(); }
};