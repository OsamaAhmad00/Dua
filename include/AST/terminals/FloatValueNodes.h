#include <AST/terminals/ValueNode.h>
#include <types/FloatTypes.h>

#define DEFINE_FLOAT_TYPE(TYPE, WIDTH)                              \
class F##WIDTH##ValueNode : public ValueNode {                      \
    TYPE value;                                                     \
    F64Type type;                                                   \
public:                                                             \
    F##WIDTH##ValueNode() { value = 0.0; }                          \
    F##WIDTH##ValueNode(TYPE value) : value(value) {}               \
    llvm::Constant *eval() override                                 \
        { return llvm::ConstantFP::get(type.llvm_type(), value); }  \
    F64Type *get_type() override { return &type; }                  \
};

DEFINE_FLOAT_TYPE(double, 64)
DEFINE_FLOAT_TYPE(float, 32)