#include <AST/terminals/ValueNode.h>

#define INTEGER_VALUE_NODE(NAME, WIDTH, TYPE)     \
class NAME : public ValueNode                     \
{                                                 \
    TYPE value;                                   \
                                                  \
public:                                           \
                                                  \
    NAME() { value = 0; }                         \
    NAME(TYPE value) : value(value) {}            \
                                                  \
    llvm::Constant* eval() override {             \
        return builder().getInt##WIDTH(value);    \
    }                                             \
                                                  \
    llvm::Constant* default_value() override {    \
        return builder().getInt##WIDTH(0);        \
    }                                             \
                                                  \
    llvm::Type* llvm_type() override {            \
        return builder().getInt##WIDTH##Ty();     \
    }                                             \
};

INTEGER_VALUE_NODE(I64ValueNode, 64, int64_t)

INTEGER_VALUE_NODE(I32ValueNode, 32, int32_t)

INTEGER_VALUE_NODE(I16ValueNode, 16, int16_t)

INTEGER_VALUE_NODE(I8ValueNode , 8 , int8_t )

INTEGER_VALUE_NODE(U64ValueNode, 64, uint64_t)

INTEGER_VALUE_NODE(U32ValueNode, 32, uint32_t)

INTEGER_VALUE_NODE(U16ValueNode, 16, uint16_t)

INTEGER_VALUE_NODE(U8ValueNode,  8 , uint8_t )
