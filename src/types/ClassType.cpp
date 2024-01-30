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

    auto& f = fields();
    for (int i = 0 ; i < initializers.size(); i++) {
        initializers[i] = f[i].type->default_value().get_constant();
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
    compiler->report_error("Class " + this->name + " doesn't contain a member with the name " + name);

    // Unreachable
    return fields().front();
}

Value ClassType::get_field(const Value& instance, const std::string &name) const {
    for (size_t i = 0; i < fields().size(); i++) {
        if (fields()[i].name == name)
            return get_field(instance, i);
    }
    compiler->report_error("Class " + this->name + " doesn't contain a member with the name " + name);
    return {};
}

Value ClassType::get_field(const Value& instance, size_t index) const
{
    // TODO avoid creating a GEP instruction on each access. You might cache the
    //  result of the access for each instance-index pair, yet, you have to bear
    //  in mind that in different basic blocks or functions, you have to perform
    //  the access again, so, it's not just a simple caching problem.
    auto& f = fields()[index];
    auto result = compiler->get_builder()->CreateStructGEP(llvm_type(), instance.get(), index, f.name);
    return compiler->create_value(result, f.type);
}

int ClassType::ancestor_distance(const ClassType *ancestor) const {
    auto distance = 0;
    auto parent = this;
    while (true) {
        auto it = compiler->name_resolver.parent_classes.find(parent->name);
        if (it == compiler->name_resolver.parent_classes.end())
            break;
        parent = it->second;
        distance++;
        if (parent == ancestor)
            return distance;
    }
    return -1;
}

Value ClassType::get_method(const std::string& name, Value instance, const std::vector<const Type*>& arg_types, bool panic_on_error) const
{
    // Load the symbol table. The symbol table may be of a child class.
    // The type tho, will be considered of the current class
    auto vtable = compiler->name_resolver.get_vtable_instance(this->name);
    if (!vtable->has_method(name)) {
        if (panic_on_error)
            report_error("The class " + to_string() + " has no method with the name " + name);
        return {};
    }
    auto vtable_ptr_ptr = get_field(instance, ".vtable_ptr");
    auto vtable_type = compiler->name_resolver.get_vtable_type(this->name)->llvm_type();
    auto vtable_ptr = compiler->builder.CreateLoad(vtable_type, vtable_ptr_ptr.get(), ".vtable");
    auto full_name = compiler->name_resolver.get_winning_method(this, name, arg_types);
    auto method_type = compiler->name_resolver.get_function(full_name, arg_types).type;
    auto method_ptr = (name != "constructor") ? vtable->get_method(full_name, method_type->llvm_type()->getPointerTo(), vtable_ptr)
                : compiler->module.getFunction(full_name);
    auto method = compiler->create_value(method_ptr, method_type);
    return method;
}

}
