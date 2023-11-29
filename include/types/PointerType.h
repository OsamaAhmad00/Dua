#pragma once

#include <types/TypeBase.h>
#include <llvm/IR/DerivedTypes.h>

namespace dua
{

class PointerType : public TypeBase
{
    TypeBase* element_type;

public:

    PointerType(ModuleCompiler* compiler, TypeBase* element_type)
            : element_type(element_type) { this->compiler = compiler; }

    llvm::Constant* default_value() override;

    llvm::PointerType * llvm_type() const override;

    TypeBase* get_element_type() { return element_type; }

    PointerType* clone() override;

    std::string to_string() const override { return element_type->to_string() + "*"; }

    ~PointerType() override { delete element_type; }
};

}
