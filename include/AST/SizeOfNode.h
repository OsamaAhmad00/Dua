#pragma once

#include <AST/ASTNode.h>
#include <types/IntegerTypes.h>

namespace dua
{

class SizeOfNode : public ASTNode
{

    Type* target_type;

public:

    SizeOfNode(ModuleCompiler* compiler, Type* target_type)
            : target_type(target_type)
    {
        this->compiler = compiler;
    }

    llvm::Value* eval() override
    {
        llvm::DataLayout dl(&module());

        auto type = target_type->llvm_type();
        if (type == nullptr) {
            // This can happen for example in the case sizeof(x).
            //  This will leave the parser confused about whether
            //  x is a variable name (an expression), or a class
            //  type. If the type is nullptr, this means that this
            //  is not a valid class type, thus, this is a variable.
            auto cls = dynamic_cast<ClassType*>(target_type);
            if (cls == nullptr)
                report_internal_error("sizeof operator called on an invalid type");
            type = symbol_table().get(cls->name).type->llvm_type();
        }

        long long size = (type->isSized()) ? dl.getTypeSizeInBits(type) / 8 : 0;
        return builder().getInt64(size);
    }

    Type* compute_type() override {
        if (type == nullptr) type = compiler->create_type<I64Type>();
        return type;
    }

    ~SizeOfNode() override {
        delete target_type;
    }
};

}
