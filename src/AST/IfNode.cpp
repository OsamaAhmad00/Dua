#include <AST/IfNode.hpp>
#include <types/VoidType.hpp>
#include <utils/ErrorReporting.hpp>

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
        llvm::BasicBlock* condition_block = create_basic_block(
                operation_name + std::to_string(counter) + "_condition" + std::to_string(i),
                current_function()
        );

        llvm::BasicBlock* branch_block = create_basic_block(
                operation_name + std::to_string(counter) + "_branch" + std::to_string(i),
                current_function()
        );

        jump_to_blocks.push_back(condition_block);
        body_blocks.push_back(branch_block);
    }

    llvm::BasicBlock* else_block = nullptr;

    if (has_else())
    {
         else_block = create_basic_block(
                operation_name + std::to_string(counter) + "_branch_else",
                current_function()
        );

        jump_to_blocks.push_back(else_block);
    }

    builder().SetInsertPoint(old_block);
    builder().CreateBr(jump_to_blocks.front());

    llvm::BasicBlock* end_block  = create_basic_block(
            operation_name + std::to_string(counter) + "_end",
            current_function()
    );
    jump_to_blocks.push_back(end_block);

    for (size_t i = 0; i < body_blocks.size(); i++)
    {
        builder().SetInsertPoint(jump_to_blocks[i]);
        auto condition = conditions[i]->eval().cast_as_bool();
        if (condition.is_null())
            report_error("The provided condition can't be casted to boolean value.");
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

    // A quick hack to include the else block in the loop
    if (else_block) body_blocks.push_back(else_block);
    for (size_t i = 0; i < body_blocks.size(); i++) {
        builder().SetInsertPoint(body_blocks[i]);
        auto value = branches[i]->eval();
        if (builder().GetInsertBlock()->empty() || !builder().GetInsertBlock()->back().isTerminator())
            builder().CreateBr(end_block);
        if (is_expression) {
            values.push_back(compiler->create_value(value.get(), branches[i]->get_type()));
            phi_blocks.push_back(builder().GetInsertBlock());
        }
    }

    builder().SetInsertPoint(end_block);

    if (!is_expression)
        return none_value();

    auto type = get_type();
    for (auto& value : values) {
        auto casted = typing_system().cast_value(value, type);
        if (casted.is_null()) {
            report_error("Mismatch in the types of the branches");
        }
    }

    llvm::PHINode* phi = builder().CreatePHI(
        values.front().get()->getType(),
        values.size(),
        operation_name + std::to_string(counter) + "_result"
    );

    for (size_t i = 0; i < values.size(); i++) {
        phi->addIncoming(values[i].get(), phi_blocks[i]);
    }

    return compiler->create_value(phi, get_type());
}

const Type *IfNode::get_type() {
    if (type != nullptr) return type;
    if (is_expression) return type = branches.front()->get_type();
    return type = compiler->create_type<VoidType>();
}

}
