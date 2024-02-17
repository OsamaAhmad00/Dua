#include "AST/loops/ForNode.hpp"

namespace dua
{

int ForNode::_counter = 0;

NoneValue ForNode::eval()
{
    name_resolver().push_scope();

    for (ASTNode* node : initializations)
        node->eval();

    int counter = _counter++;  // storing a local copy
    // You might delay the attachment of the blocks to the functions to reorder the blocks in a more readable way.
    llvm::BasicBlock* update_block = create_basic_block("for_update" + std::to_string(counter), current_function());
    llvm::BasicBlock* cond_block = create_basic_block("for_cond" + std::to_string(counter), current_function());
    llvm::BasicBlock* body_block = create_basic_block("for_body" + std::to_string(counter), current_function());
    llvm::BasicBlock* end_block = create_basic_block("for_end" + std::to_string(counter), current_function());

    continue_stack().push_back(update_block);
    break_stack().push_back(end_block);

    // From the current block.
    builder().CreateBr(cond_block);

    builder().SetInsertPoint(update_block);
    update_exp->eval();
    if (builder().GetInsertBlock()->empty() || !builder().GetInsertBlock()->back().isTerminator())
        builder().CreateBr(cond_block);

    builder().SetInsertPoint(cond_block);
    auto cond_res = cond_exp->eval().cast_as_bool();
    if (cond_res.is_null())
        compiler->report_error("The provided condition can't be casted to boolean value.");
    builder().CreateCondBr(cond_res.get(), body_block, end_block);

    builder().SetInsertPoint(body_block);
    body_exp->eval();
    if (builder().GetInsertBlock()->empty() || !builder().GetInsertBlock()->back().isTerminator())
        builder().CreateBr(update_block);

    builder().SetInsertPoint(end_block);

    continue_stack().pop_back();
    break_stack().pop_back();

    name_resolver().pop_scope();

    return none_value();
}

}
