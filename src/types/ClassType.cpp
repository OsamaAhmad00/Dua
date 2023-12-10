#include <types/ClassType.hpp>
#include <llvm/IR/Constants.h>
#include <ModuleCompiler.hpp>

namespace dua
{

ClassType::ClassType(ModuleCompiler *compiler, std::string name)
        : name(std::move(name))
{
    this->compiler = compiler;

    // Just to make sure the type is declared before usage.
    llvm::StructType::create(compiler->context, this->name);
}

llvm::Constant *ClassType::default_value() const
{
    std::vector<llvm::Constant*> initializers(fields().size());

    for (int i = 0 ; i < initializers.size(); i++) {
        initializers[i] = fields()[i].default_value;
    }

    return llvm::ConstantStruct::get(llvm_type(), std::move(initializers));
}

llvm::StructType* ClassType::llvm_type() const {
    return llvm::StructType::getTypeByName(*compiler->get_context(), name);
}

const std::vector<ClassField> &ClassType::fields() const {
    return compiler->name_resolver.class_fields[name];
}

const ClassField& ClassType::get_field(const std::string &name) const {
    for (auto & field : fields()) {
        if (field.name == name)
            return field;
    }
    report_error("Class " + this->name + " doesn't contain a member with the name " + name);

    // Unreachable
    return fields().front();
}

llvm::Value *ClassType::get_field(llvm::Value *instance, const std::string &name) const {
    for (size_t i = 0; i < fields().size(); i++) {
        if (fields()[i].name == name)
            return get_field(instance, i);
    }
    report_error("Class " + this->name + " doesn't contain a member with the name " + name);
    return nullptr;
}

llvm::Value *ClassType::get_field(llvm::Value *instance, size_t index) const {
    return compiler->get_builder()->CreateStructGEP(llvm_type(), instance, index, fields()[index].name);
}

}
