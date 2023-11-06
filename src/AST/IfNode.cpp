#include <AST/IfNode.h>

int IfNode::_counter = 0;

llvm::PHINode* IfNode::eval()
{
    int counter = _counter++;  // storing a local copy
    // You might delay the attachment of the blocks to the functions to reorder the blocks in a more readable way
    //  by doing the following: current_function->getBasicBlockList().push_back(block); I prefer this way.
    llvm::BasicBlock* then_block = create_basic_block("then" + std::to_string(counter), current_function());
    llvm::BasicBlock* else_block = create_basic_block("else" + std::to_string(counter), current_function());
    llvm::BasicBlock* end_block  = create_basic_block("if_end" + std::to_string(counter), current_function());


    llvm::Value* cond_res = cond_exp->eval();
    builder().CreateCondBr(cond_res, then_block, else_block);

    builder().SetInsertPoint(then_block);
    llvm::Value* then_res = then_exp->eval();
    // Since the block may contain nested statements, the current then_block may be terminated early, and after the eval
    //  of the then_exp, we end up in a completely different block. This block is where the branching instruction to
    //  the end_block will be inserted, and this block is the block that would be a predecessor of the end_block, instead
    //  of the current then_block. Thus, we need to use the block we ended up upon after evaluating the then_exp instead
    //  of the current then_block (the two blocks may be the same) when referring to the phi instruction, which expects
    //  the provided blocks to be its predecessor.
    builder().CreateBr(end_block);
    then_block = builder().GetInsertBlock();

    builder().SetInsertPoint(else_block);
    llvm::Value* else_res = else_exp->eval();
    // Same as the then block.
    builder().CreateBr(end_block);
    else_block = builder().GetInsertBlock();

    assert(then_res->getType() == else_res->getType());
    builder().SetInsertPoint(end_block);

    llvm::PHINode* phi = builder().CreatePHI(then_res->getType(), 2, "if_result" + std::to_string(counter));
    phi->addIncoming(then_res, then_block);
    phi->addIncoming(else_res, else_block);

    return phi;
}

void IfNode::set_else(ASTNode *node)
{
    assert(else_exp == nullptr);
    else_exp = node;
}

IfNode &IfNode::add_else_if(ASTNode *condition, ASTNode *node)
{
    auto x = new IfNode(condition, node, this->else_exp);
    this->else_exp = x;
    return *x;
}

IfNode::~IfNode()
{
    delete cond_exp;
    delete then_exp;
    delete else_exp;
}