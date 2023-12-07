#include "AST/class/ClassDefinitionNode.hpp"
#include "AST/variable/LocalVariableDefinitionNode.hpp"
#include "AST/function/FunctionDefinitionNode.hpp"
#include "utils/TextManipulation.hpp"

namespace dua
{

size_t put_fields_first(std::vector<ASTNode*>& members)
{
    // Returns the split point between fields and methods,
    //  pointing to the first method in the list.
    // Fields are put at the beginning, in order of definition
    size_t result = -1;
    for (size_t i = 0; i < members.size(); i++) {
        if (i == result) continue;
        if (dynamic_cast<LocalVariableDefinitionNode*>(members[i]) != nullptr)
            std::swap(members[i--], members[++result]);
    }
    return result + 1;
}

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

    // Put fields at the beginning of the list. This is in order to define
    //  the fields first, so that they're visible in all methods. Remember
    //  that by the time of evaluation of any node, all functions and methods
    //  are already declared and are visible, so, nothing special needs to
    //  be done for them.
    // TODO enforce an order of fields definition, for example, define them in
    //  the order of definition in the class
    size_t split_point = put_fields_first(members);

    if (current_class()->isSized())
        report_error("Redefinition of a class");

    // A non-const direct access
    auto &fields = name_resolver().class_fields[name];
    fields.reserve(split_point);

    // Evaluate fields first
    // The fields will register themselves
    for (size_t i = 0; i < split_point; i++)
        members[i]->eval();

    std::vector<llvm::Type*> body(split_point);
    for (size_t i = 0; i < split_point; i++)
        body[i] = fields[i].type->llvm_type();
    current_class()->setBody(body, is_packed);

    // Now evaluate methods
    for (size_t i = split_point; i < members.size(); i++) {
        auto& func_name = ((FunctionDefinitionNode*)members[i])->name;
        if (ends_with(func_name, ".constructor")) {
            compiler->push_deferred_node(members[i]);
            continue;
        }
        members[i]->eval();
    }

    current_class() = old_class;
    current_function() = old_function;

    return result = none_value();
}

}