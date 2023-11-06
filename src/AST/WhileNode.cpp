#include <AST/WhileNode.h>

int WhileNode::_counter = 0;

NoneValue WhileNode::eval()
{
    int counter = _counter++;  // storing a local copy
    // You might delay the attachment of the blocks to the functions to reorder the blocks in a more readable way
    //  by doing the following: current_function->getBasicBlockList().push_back(block); I prefer this way.
    llvm::BasicBlock* cond_block = create_basic_block("while_cond" + std::to_string(counter), current_function());
    llvm::BasicBlock* body_block = create_basic_block("while_body" + std::to_string(counter), current_function());
    llvm::BasicBlock* end_block = create_basic_block("while_end" + std::to_string(counter), current_function());

    // From the current block.
    builder().CreateBr(cond_block);

    builder().SetInsertPoint(cond_block);
    llvm::Value* cond_res = cond_exp->eval();
    builder().CreateCondBr(cond_res, body_block, end_block);

    builder().SetInsertPoint(body_block);
    body_exp->eval();
    builder().CreateBr(cond_block);

    builder().SetInsertPoint(end_block);

    return none_value();
}

WhileNode::~WhileNode()
{
    delete cond_exp;
    delete body_exp;
}