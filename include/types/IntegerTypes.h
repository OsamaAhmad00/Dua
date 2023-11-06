#include <types/TypeBase.h>

#define DEFINE_INTEGER_TYPE(PREFIX, WIDTH)      \
struct PREFIX##WIDTH##Type : public TypeBase    \
{                                               \
    llvm::Constant* default_value() override {  \
        return builder->getInt##WIDTH(0);       \
    }                                           \
                                                \
    llvm::Type* llvm_type() override {          \
        return builder->getInt##WIDTH##Ty();    \
    }                                           \
};


DEFINE_INTEGER_TYPE(I, 64)
DEFINE_INTEGER_TYPE(I, 32)
DEFINE_INTEGER_TYPE(I, 16)
DEFINE_INTEGER_TYPE(I, 8)

DEFINE_INTEGER_TYPE(U, 64)
DEFINE_INTEGER_TYPE(U, 32)
DEFINE_INTEGER_TYPE(U, 16)
DEFINE_INTEGER_TYPE(U, 8)
