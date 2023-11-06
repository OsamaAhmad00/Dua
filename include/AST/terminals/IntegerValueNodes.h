#include <AST/terminals/ValueNode.h>
#include <types/IntegerTypes.h>

#define DEFINE_INTEGER_VALUE_NODE(PREFIX, WIDTH, TYPE)          \
class PREFIX##WIDTH##ValueNode : public ValueNode               \
{                                                               \
    TYPE value;                                                 \
    PREFIX##WIDTH##Type type;                                   \
                                                                \
public:                                                         \
                                                                \
    PREFIX##WIDTH##ValueNode() { value = 0; }                   \
    PREFIX##WIDTH##ValueNode(TYPE value)                        \
        : value(value) {}                                       \
                                                                \
    llvm::Constant* eval() override {                           \
        return builder().getInt##WIDTH(value);                  \
    }                                                           \
                                                                \
    PREFIX##WIDTH##Type* get_type() override { return &type; }  \
};

DEFINE_INTEGER_VALUE_NODE(I, 64, int64_t)
DEFINE_INTEGER_VALUE_NODE(I, 32, int32_t)
DEFINE_INTEGER_VALUE_NODE(I, 16, int16_t)
DEFINE_INTEGER_VALUE_NODE(I, 8,  int8_t)

DEFINE_INTEGER_VALUE_NODE(U, 64, uint64_t)
DEFINE_INTEGER_VALUE_NODE(U, 32, uint32_t)
DEFINE_INTEGER_VALUE_NODE(U, 16, uint16_t)
DEFINE_INTEGER_VALUE_NODE(U, 8,  uint8_t)
