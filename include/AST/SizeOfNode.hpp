#pragma once

#include <AST/ASTNode.hpp>
#include <types/IntegerTypes.hpp>

namespace dua
{

class SizeOfNode : public ASTNode
{

    const Type* target_type;

public:

    SizeOfNode(ModuleCompiler* compiler, const Type* target_type)
            : target_type(target_type)
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        llvm::DataLayout dl(&module());

        auto type = target_type->llvm_type();
        if (type == nullptr) {
            // This can happen for example in the case sizeof(x).
            //  This will leave the parser confused about whether
            //  x is a variable name (an expression), or a class
            //  type. If the type is nullptr, this means that this
            //  is not a valid class type, thus, this is a variable.
            auto cls = target_type->as<ClassType>();
            if (cls == nullptr)
                report_internal_error("sizeof operator called on an invalid type");
            type = name_resolver().symbol_table.get(cls->name).type->llvm_type();
        }

        long long size = (type->isSized()) ? dl.getTypeSizeInBits(type) / 8 : 0;
        return compiler->create_value(builder().getInt64(size), get_type());
    }

    const Type* get_type() override {
        if (type == nullptr) type = compiler->create_type<I64Type>();
        return type;
    }
};

}
