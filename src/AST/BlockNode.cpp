#include <AST/BlockNode.hpp>
#include "AST/lvalue/VariableNode.hpp"
#include "AST/variable/LocalVariableDefinitionNode.hpp"
#include "AST/operators/MoveNode.hpp"
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

    if (!result.is_null() && result.type->is<ClassType>())
    {
        compiler->remove_temp_expr(result.id);

        auto result_ptr = builder().CreateAlloca(result.type->llvm_type(), nullptr, "block_value");
        auto ptr_value = compiler->create_value(result_ptr, result.type);

        // This is to force the loading of the returned value before
        //  returning it, to avoid returning stale versions.
        if (result.memory_location != nullptr)
            result.set(nullptr);

        name_resolver().copy_construct(ptr_value, result);

        // Moving the address to the memory_location field
        result.memory_location = result_ptr;
        result.set(nullptr);
        result.id = compiler->get_temp_expr_map_unused_id();

        compiler->insert_temp_expr(result);
    }

    compiler->destruct_last_scope();
    compiler->pop_scope();

    return result;
}

const Type* BlockNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    if (elements.empty()) return compiler->create_type<VoidType>();

    auto back = elements.back();

    std::string lookup_name;

    if (auto var = back->as<VariableNode>(); var != nullptr)
        lookup_name = var->unresolved_name->resolve();
    else if (auto move = back->as<MoveNode>(); move != nullptr)
        lookup_name = move->get_name();

    if (!lookup_name.empty())
    {
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
        for (size_t i = 0; i < elements.size() - 1; i++) {
            if (auto local = elements[i]->as<LocalVariableDefinitionNode>(); local != nullptr) {
                // Since the same name can't be defined
                //  more than once in the same scope, once
                //  we find the name, return the type.
                if (lookup_name == local->name)
                    return set_type(local->get_definition_type());
            }
        }
    }

    return back->get_type();
}

}
