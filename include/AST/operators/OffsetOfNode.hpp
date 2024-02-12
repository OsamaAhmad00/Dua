#pragma once

#include <AST/ASTNode.hpp>
#include <types/IntegerTypes.hpp>

namespace dua
{

class OffsetOfNode : public ASTNode
{
    const Type* unresolved_class_type;
    std::string field_name;

public:

    OffsetOfNode(ModuleCompiler* compiler, const Type* class_type, std::string field_name)
            : unresolved_class_type(class_type), field_name(std::move(field_name))
    {
        this->compiler = compiler;
    }

    Value eval() override
    {
        auto class_type = unresolved_class_type->get_concrete_type()->as<ClassType>();
        if (class_type == nullptr) {
            report_error(
                "The offsetof operator takes only a class type"
                " or an expression that evaluates to a class type. Got "
                + unresolved_class_type->to_string() + " instead."
            );
        }

        auto& fields = class_type->fields();
        // 0 is invalid index since it points to the vtable pointer, which is an implementation details
        size_t index = 0;
        for (size_t i = 0; i < fields.size(); i++) {
            if (fields[i].name == field_name) {
                index = i;
                break;
            }
        }

        if (index == 0)
            report_error("The class " + class_type->name + " has no field with the name " + field_name);

        auto type = class_type->llvm_type();
        auto null = builder().CreateIntToPtr(builder().getInt64(0), type->getPointerTo());
        auto result = builder().CreateStructGEP(type, null, index);
        result = builder().CreatePtrToInt(result, builder().getInt64Ty());

        return compiler->create_value(result, get_type());
    }

    const Type* get_type() override {
        if (type == nullptr) type = compiler->create_type<I64Type>();
        return type;
    }
};

}
