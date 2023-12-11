#include "AST/class/ClassDefinitionNode.hpp"
#include "AST/variable/ClassFieldDefinitionNode.hpp"
#include "AST/function/FunctionDefinitionNode.hpp"
#include "utils/TextManipulation.hpp"

namespace dua
{

llvm::Value *ClassDefinitionNode::eval()
{
    if (result != nullptr)
        return result;

    if (name_resolver().classes.find(name) == name_resolver().classes.end())
        report_internal_error("Definition of the class " + name + " before registering it");

    auto old_class = current_class();
    current_class() = llvm::StructType::getTypeByName(context(), name);

    // If current function is not nullptr, the variable definitions
    //  will be considered a local variable of that function.
    auto old_function = current_function();
    current_function() = nullptr;

    // Evaluate fields first
    // The fields will register themselves
    for (auto & field : fields)
        field->eval();

    // Now evaluate methods
    for (auto & method : methods) {
        auto& func_name = method->name;
        if (ends_with(func_name, ".constructor"))
            compiler->push_deferred_node(method);
        else
            method->eval();
    }

    current_class() = old_class;
    current_function() = old_function;

    return result = none_value();
}

}