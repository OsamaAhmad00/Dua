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
    if (llvm::StructType::getTypeByName(*compiler->get_context(), this->name) == nullptr)
        llvm::StructType::create(compiler->context, this->name);
}

Value ClassType::default_value() const
{
    std::vector<llvm::Constant*> initializers(fields().size());

    for (int i = 0 ; i < initializers.size(); i++) {
        initializers[i] = fields()[i].default_value;
    }

    auto result = llvm::ConstantStruct::get(llvm_type(), std::move(initializers));
    return compiler->create_value(result, this);
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

Value ClassType::get_field(const Value& instance, const std::string &name) const {
    for (size_t i = 0; i < fields().size(); i++) {
        if (fields()[i].name == name)
            return get_field(instance, i);
    }
    report_error("Class " + this->name + " doesn't contain a member with the name " + name);
    return {};
}

Value ClassType::get_field(const Value& instance, size_t index) const
{
    // TODO avoid creating a GEP instruction on each access. You might cache the
    //  result of the access for each instance-index pair, yet, you have to bear
    //  in mind that in different basic blocks or functions, you have to perform
    //  the access again, so, it's not just a simple caching problem.
    auto result = compiler->get_builder()->CreateStructGEP(llvm_type(), instance.get(), index, fields()[index].name);
    return compiler->create_value(result, fields()[index].type);
}

}
