#include <AST/terminals/ValueNode.h>
#include <types/IntegerTypes.h>

#define DEFINE_INTEGER_VALUE_NODE(WIDTH)                   \
class I##WIDTH##ValueNode : public ValueNode               \
{                                                          \
    int##WIDTH##_t value;                                  \
    I##WIDTH##Type type;                                   \
                                                           \
public:                                                    \
                                                           \
    I##WIDTH##ValueNode() { value = 0; }                   \
    I##WIDTH##ValueNode(int##WIDTH##_t value)              \
        : value(value) {}                                  \
                                                           \
    llvm::Constant* eval() override {                      \
        return builder().getInt##WIDTH(value);             \
    }                                                      \
                                                           \
    I##WIDTH##Type* get_type() override { return &type; }  \
};

DEFINE_INTEGER_VALUE_NODE(64)
DEFINE_INTEGER_VALUE_NODE(32)
DEFINE_INTEGER_VALUE_NODE(16)
DEFINE_INTEGER_VALUE_NODE(8)
