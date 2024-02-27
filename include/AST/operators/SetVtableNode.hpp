#pragma once

#include <AST/ASTNode.hpp>

namespace dua
{

class SetVtableNode : public ASTNode
{
    ASTNode* expr;
    const Type* target;

public:

    SetVtableNode(ModuleCompiler* compiler, ASTNode* expr, const Type* target) : expr(expr), target(target)
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        // In case the provided arguments are not of a class type, just return. This is
        //  to allow templated classes to use it without having to ensure the target type

        auto class_type = target->get_concrete_type()->is<ClassType>();

        if (class_type == nullptr) return none_value();

        auto instance = expr->eval();

        if (instance.memory_location == nullptr)
            compiler->report_error("Can't set the vtable of an expression of type " + target->to_string() + " that has no memory address");

        if (instance.type->as<ClassType>() == nullptr) return none_value();

        auto vtable_instance = name_resolver().get_vtable_instance(class_type->name)->instance;

        instance.set(instance.memory_location);
        instance.memory_location = nullptr;
        auto ptr = class_type->get_field(instance, ".vtable_ptr").get();

        builder().CreateStore(vtable_instance, ptr);

        return none_value();
    }
};

}
