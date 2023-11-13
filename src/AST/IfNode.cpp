#include <AST/IfNode.h>

int IfNode::_counter = 0;

llvm::PHINode* IfNode::eval()
{
    // 1 - Create all blocks, and start chaining from
    //      the most bottom block (end_if)
    // 2 - Evaluate the conditions of the branches and
    //      create a conditional branch
    // 3 - After all blocks are created, attach the blocks
    //      to the current_function() in an appropriate order
    // 4 - Evaluate each branch inside its corresponding
    //      block
    // 5 - Cast the results of all blocks to the type of
    //      the result of the first block
    // 6 - Create a PHI node, passing the corresponding
    //      values and blocks.

    assert(!conditions.empty());

    int counter = _counter++;  // storing a local copy

    // Will be used later to insert a branch instruction at the end of it
    llvm::BasicBlock* old_block = builder().GetInsertBlock();

    llvm::BasicBlock* end_block  = create_basic_block(
        "if" + std::to_string(counter) + "_end",
        nullptr
    );

    std::vector<llvm::BasicBlock*> branch_first_blocks { end_block };
    std::vector<llvm::BasicBlock*> branch_second_blocks;
    std::vector<llvm::BasicBlock*> phi_blocks;
    std::vector<llvm::Value*> values;

    llvm::BasicBlock* else_block = nullptr;

    if (has_else())
    {
        else_block = create_basic_block(
            "if" + std::to_string(counter) + "_branch_else",
            nullptr
        );
        branch_first_blocks.push_back(else_block);
    }

    for (size_t i = conditions.size() - 1; i != (size_t)-1; i--)
    {
        llvm::BasicBlock* condition_block = create_basic_block(
                "if" + std::to_string(counter) + "_condition" + std::to_string(i),
                nullptr
        );

        llvm::BasicBlock* branch_block = create_basic_block(
                "if" + std::to_string(counter) + "_branch" + std::to_string(i),
                nullptr
        );

        builder().SetInsertPoint(condition_block);
        llvm::Value* condition = conditions[i]->eval();
        condition = compiler->cast_value(condition, builder().getInt1Ty());
        if (condition == nullptr)
            throw std::runtime_error("The provided condition can't be casted to boolean value.");
        builder().CreateCondBr(condition, branch_block, branch_first_blocks.back());

        branch_first_blocks.push_back(condition_block);
        branch_second_blocks.push_back(branch_block);
    }

    std::reverse(branch_first_blocks.begin(), branch_first_blocks.end());
    std::reverse(branch_second_blocks.begin(), branch_second_blocks.end());

    for (size_t i = 0; i < branch_second_blocks.size(); i++) {
        current_function()->getBasicBlockList().push_back(branch_first_blocks[i]);
        current_function()->getBasicBlockList().push_back(branch_second_blocks[i]);
    }

    for (size_t i = branch_second_blocks.size(); i < branch_first_blocks.size(); i++) {
        // else_block if found, and end_block
        current_function()->getBasicBlockList().push_back(branch_first_blocks[i]);
    }

    // A quick hack to include the else_block in the loop below
    if (else_block) branch_second_blocks.push_back(else_block);

    for (size_t i = 0; i < branch_second_blocks.size(); i++) {
        builder().SetInsertPoint(branch_second_blocks[i]);
        values.push_back(branches[i]->eval());
        builder().CreateBr(end_block);
        phi_blocks.push_back(builder().GetInsertBlock());
    }

    for (size_t i = 1; i < values.size(); i++) {
        values[i] = compiler->cast_value(values[i], values.front()->getType());
        if (values[i] == nullptr) {
            throw std::runtime_error("Mismatch in the types of the branches");
        }
    }

    builder().SetInsertPoint(old_block);
    builder().CreateBr(branch_first_blocks.front());

    builder().SetInsertPoint(end_block);
    llvm::PHINode* phi = builder().CreatePHI(
        values.front()->getType(),
        values.size(),
        "if" + std::to_string(counter) + "_result"
    );

    for (size_t i = 0; i < values.size(); i++) {
        phi->addIncoming(values[i], phi_blocks[i]);
    }

    return phi;
}


IfNode::~IfNode()
{
    for (auto ptr : conditions)
        delete ptr;
    for (auto ptr : branches)
        delete ptr;
}
