#include "AST/loops/DoWhileNode.hpp"
#include <utils/ErrorReporting.hpp>

namespace dua
{

int DoWhileNode::_counter = 0;

NoneValue DoWhileNode::eval()
{
    int counter = _counter++;  // storing a local copy
    // You might delay the attachment of the blocks to the functions to reorder the blocks in a more readable way.
    llvm::BasicBlock* body_block = create_basic_block("do_while_body" + std::to_string(counter), current_function());
    llvm::BasicBlock* cond_block = create_basic_block("do_while_cond" + std::to_string(counter), current_function());
    llvm::BasicBlock* end_block = create_basic_block("do_while_end" + std::to_string(counter), current_function());

    continue_stack().push_back(cond_block);
    break_stack().push_back(end_block);

    // From the current block.
    builder().CreateBr(body_block);

    builder().SetInsertPoint(cond_block);
    auto cond_res = cond_exp->eval().cast_as_bool();
    if (cond_res.is_null())
        report_error("The provided condition can't be casted to boolean value.");
    builder().CreateCondBr(cond_res.get(), body_block, end_block);

    builder().SetInsertPoint(body_block);
    body_exp->eval();
    if (builder().GetInsertBlock()->empty() || !builder().GetInsertBlock()->back().isTerminator())
        builder().CreateBr(cond_block);

    builder().SetInsertPoint(end_block);

    continue_stack().pop_back();
    break_stack().pop_back();

    return none_value();
}

}
