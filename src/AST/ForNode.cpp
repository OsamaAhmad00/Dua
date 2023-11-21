#include "AST/loops/ForNode.h"
#include <utils/ErrorReporting.h>

namespace dua
{

int ForNode::_counter = 0;

NoneValue ForNode::eval()
{
    symbol_table().push_scope();

    for (ASTNode* node : initializations)
        node->eval();

    int counter = _counter++;  // storing a local copy
    // You might delay the attachment of the blocks to the functions to reorder the blocks in a more readable way
    //  by doing the following: current_function->getBasicBlockList().push_back(block); I prefer this way.
    llvm::BasicBlock* update_block = create_basic_block("for_update" + std::to_string(counter), current_function());
    llvm::BasicBlock* cond_block = create_basic_block("for_cond" + std::to_string(counter), current_function());
    llvm::BasicBlock* body_block = create_basic_block("for_body" + std::to_string(counter), current_function());
    llvm::BasicBlock* end_block = create_basic_block("for_end" + std::to_string(counter), current_function());

    compiler->get_continue_stack().push_back(update_block);
    compiler->get_break_stack().push_back(end_block);

    // From the current block.
    builder().CreateBr(cond_block);

    builder().SetInsertPoint(update_block);
    update_exp->eval();
    if (builder().GetInsertBlock()->empty() || !builder().GetInsertBlock()->back().isTerminator())
        builder().CreateBr(cond_block);

    builder().SetInsertPoint(cond_block);
    llvm::Value* cond_res = cond_exp->eval();
    cond_res = compiler->cast_value(cond_res, builder().getInt1Ty());
    if (cond_res == nullptr)
        report_error("The provided condition can't be casted to boolean value.");
    builder().CreateCondBr(cond_res, body_block, end_block);

    builder().SetInsertPoint(body_block);
    body_exp->eval();
    if (builder().GetInsertBlock()->empty() || !builder().GetInsertBlock()->back().isTerminator())
        builder().CreateBr(update_block);

    builder().SetInsertPoint(end_block);

    compiler->get_continue_stack().pop_back();
    compiler->get_break_stack().pop_back();

    symbol_table().push_scope();

    return none_value();
}

ForNode::~ForNode()
{
    for (auto exp : initializations)
        delete exp;
    delete cond_exp;
    delete body_exp;
    delete update_exp;
}

}
