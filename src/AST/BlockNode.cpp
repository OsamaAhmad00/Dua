#include <AST/BlockNode.hpp>
#include "AST/lvalue/VariableNode.hpp"
#include "AST/variable/LocalVariableDefinitionNode.hpp"
#include <types/VoidType.hpp>

namespace dua
{

BlockNode& BlockNode::append(ASTNode *element)
{
    elements.push_back(element);
    return *this;
}

Value BlockNode::eval()
{
    if (elements.empty())
        return none_value();

    compiler->push_scope();

    for (size_t i = 0; i < elements.size() - 1; i++) {
        elements[i]->eval();
    }

    auto result =  elements.back()->eval();

    auto return_type = result.type;
    // dynamic_cast is used here because the type of the result
    //  might be a nullptr if the block is not a block expression.
    //  Also, because Type::as ignores references, which we don't
    //  want to ignore
    auto class_type = dynamic_cast<const ClassType*>(return_type);
    bool is_lvalue_object = class_type != nullptr && result.memory_location != nullptr;
    Value vtable_ptr;
    llvm::Value* old_vtable;
    if (is_lvalue_object)
    {
        // The get_field method expects a pointer and not the object itself
        auto instance = compiler->create_value(result.memory_location, result.type);
        vtable_ptr = class_type->get_field(instance, ".vtable_ptr");
        old_vtable = compiler->swap_vtables(vtable_ptr.get(), class_type);
    }

    compiler->destruct_last_scope();
    compiler->pop_scope();

    if (is_lvalue_object)
        compiler->restore_vtable(vtable_ptr.get(), old_vtable);

    return result;
}

const Type* BlockNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    if (elements.empty()) return compiler->create_type<VoidType>();

    auto back = elements.back();
    if (auto var = back->as<VariableNode>(); var != nullptr) {
        // Variable nodes are an exception. Variable nodes look in the
        //  symbol table for their name, and return their type. If its
        //  name is shadowed in this block, we should return the type
        //  of the new variable with the same name.
        // We don't care about the cases where the name of the variable
        //  refers to a function or a class name since these are allowed
        //  at the global scope only, and can't be shadowed here.
        // Also, if the name is not present outside of this block, we
        //  would get an undefined identifier error
        // Luckily, since the return type of the block expression is its
        //  last expression, we don't have to recurse into each of the
        //  children nodes to look up the name.
        auto name = var->unresolved_name->resolve();
        for (size_t i = 0; i < elements.size() - 1; i++) {
            if (auto local = elements[i]->as<LocalVariableDefinitionNode>(); local != nullptr) {
                // Since the same name can't be defined
                //  more than once in the same scope, once
                //  we find the name, return the type.
                if (name == local->name)
                    return set_type(local->get_definition_type());
            }
        }
    }

    return back->get_type();
}

}
