#include <AST/IfNode.hpp>
#include <types/VoidType.hpp>

namespace dua
{

int IfNode::_counter = 0;

Value IfNode::eval()
{
    assert(!conditions.empty());

    int counter = _counter++;  // storing a local copy

    // Will be used later to insert a branch instruction at the end of it
    llvm::BasicBlock* old_block = builder().GetInsertBlock();

    std::vector<llvm::BasicBlock*> jump_to_blocks;
    std::vector<llvm::BasicBlock*> body_blocks;

    for (size_t i = 0; i < conditions.size(); i++)
    {
        llvm::BasicBlock* condition_block = compiler->create_basic_block(
                operation_name + std::to_string(counter) + "_condition" + std::to_string(i)
        );

        llvm::BasicBlock* branch_block = compiler->create_basic_block(
                operation_name + std::to_string(counter) + "_branch" + std::to_string(i)
        );

        jump_to_blocks.push_back(condition_block);
        body_blocks.push_back(branch_block);
    }

    llvm::BasicBlock* else_block = nullptr;

    if (has_else())
    {
         else_block = compiler->create_basic_block(
                operation_name + std::to_string(counter) + "_branch_else"
        );

        jump_to_blocks.push_back(else_block);
    }

    builder().SetInsertPoint(old_block);
    builder().CreateBr(jump_to_blocks.front());

    llvm::BasicBlock* end_block = compiler->create_basic_block(
            operation_name + std::to_string(counter) + "_end"
    );
    jump_to_blocks.push_back(end_block);

    for (size_t i = 0; i < body_blocks.size(); i++)
    {
        builder().SetInsertPoint(jump_to_blocks[i]);
        auto condition = conditions[i]->eval().cast_as_bool();
        if (condition.is_null())
            compiler->report_error("The provided condition can't be casted to boolean value.");
        builder().CreateCondBr(condition.get(), body_blocks[i], jump_to_blocks[i + 1]);
    }

    // The conditionals may be nested. And the block that
    //  branches to the block containing the phi node
    //  might not be the same as the block the code will
    //  be evaluated inside. For this, we need to keep
    //  track of where the block each branch has ended
    //  at after the evaluation.
    std::vector<llvm::BasicBlock*> phi_blocks;
    std::vector<Value> values;

    auto type = get_type();

    bool storing_address = false;

    // A quick hack to include the else block in the loop
    if (else_block) body_blocks.push_back(else_block);
    for (size_t i = 0; i < body_blocks.size(); i++)
    {
        builder().SetInsertPoint(body_blocks[i]);
        auto value = branches[i]->eval();
        // An if expression can contain multiple branches.
        //  Only one of them should be destructed (if needed).
        //  To simplify the destruction process, if expressions
        //  remove every branch result, and inserts the final
        //  result instead.
        // If an expression returns an object, which might be
        //  copied or destructed, we'll need the address of the
        //  returned object as well. For this, we make the branches
        //  return the addresses, then make a load of the final result
        if (is_expression)
        {
            assert(!value.is_null());
            value = value.cast_as(type, false);
            if (value.is_null())
                report_error("Type mismatch between " + operation_name + " branches");
            bool _storing_address = value.type->as<ClassType>() && value.memory_location != nullptr;
            if (i != 0 && storing_address != _storing_address) {
                // Making sure that all branches are the same
                report_error("Branches of " + operation_name + " expression with different types or storage classes");
            }
            storing_address = _storing_address;
            compiler->remove_temp_expr(value.id);
            if (storing_address) {
                value.set(value.memory_location);
                value.memory_location = nullptr;
            }
            values.push_back(value);
            phi_blocks.push_back(builder().GetInsertBlock());
        }
        if (builder().GetInsertBlock()->empty() || !builder().GetInsertBlock()->back().isTerminator())
            builder().CreateBr(end_block);
    }

    builder().SetInsertPoint(end_block);

    if (!is_expression)
        return none_value();

    // We're using llvm::Value::getType here instead because the
    // dua::Type might be different if the expression is returning
    // the addresses instead of the actual value
    llvm::PHINode* phi = builder().CreatePHI(
        values.front().get()->getType(),
        values.size(),
        operation_name + std::to_string(counter) + "_result"
    );

    for (size_t i = 0; i < values.size(); i++) {
        phi->addIncoming(values[i].get(), phi_blocks[i]);
    }

    Value result;
    if (storing_address)
        result = compiler->create_value(type, phi);
    else
        result = compiler->create_value(phi, type);
    result.is_teleporting = true;
    result.id = compiler->get_temp_expr_map_unused_id();
    compiler->insert_temp_expr(result);

    return result;
}

const Type *IfNode::get_type()
{
    if (compiler->clear_type_cache) type = nullptr;

    if (type != nullptr) return type;

    if (is_expression) return set_type(branches.front()->get_type());

    return set_type(compiler->create_type<VoidType>());
}

}
