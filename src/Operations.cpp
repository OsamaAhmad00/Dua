#include "Compiler.h"

#define BINARY_OP(OP, EXP, LABEL, PARAM_RESTRICTION) \
assert(EXP.list.size() PARAM_RESTRICTION 3); \
llvm::Value* result = builder.OP(eval(EXP.list[1]), eval(EXP.list[2]), LABEL);\
for (int i = 3; i < EXP.list.size(); i++) \
    result = builder.OP(result, eval(EXP.list[i]));                    \
return result;

llvm::Function* Compiler::eval_function(const Expression& expression) {
    //              0          1         2      3  4           5
    // Definition : varfun/fun func_name params -> return_type body
    // Declaration: varfun/fun func_name params -> return_type

    assert(expression.list.size() >= 5);
    assert(expression.list[3].str == "->");

    bool is_var_arg = expression.list[0].str == "varfun";

    auto& source_params = expression.list[2].list;
    Parameters parameters;
    assert(source_params.empty() || source_params.size() == 2 || source_params.size() % 2 == 1);
    for (int i = 0; i < source_params.size(); i+=3) {
        if (i != source_params.size() - 2)
            assert(source_params[i + 2].str == ",");
        parameters.emplace_back(source_params[i].str, source_params[i+1].str);
    }

    if (expression.list.size() == 5)
        return declare_function(expression.list[1].str, expression.list[4].str, parameters, is_var_arg);
    else
        return define_function(expression.list[1].str, expression.list[5], expression.list[4].str, parameters, is_var_arg);
}

llvm::ReturnInst* Compiler::eval_return(const Expression& expression) {
    assert(expression.list.size() == 2);
    return builder.CreateRet(eval(expression.list[1]));
}

llvm::CallInst* Compiler::eval_function_call(const Expression& expression) {
    assert(expression.list.size() >= 1);
    std::vector<llvm::Value*> args(expression.list.size() - 1);
    for (int i = 1; i < expression.list.size(); i++)
        args[i - 1] = eval(expression.list[i]);
    return call_function(expression.list[0].str, args);
}

llvm::Value* Compiler::eval_scope(const Expression& expression) {
    symbol_table.push_scope();
    for (int i = 1; i < expression.list.size() - 1; i++)
        eval(expression.list[i]);
    llvm::Value* result = eval(expression.list.back());
    symbol_table.pop_scope();
    return result;
}

llvm::AllocaInst* Compiler::create_local_variable(const Expression& expression) {
    // type name (optional init)
    assert(expression.list.size() >= 2);
    llvm::Type* type = get_type(expression.list[0].str);
    llvm::Value* init = nullptr;
    if (expression.list.size() > 2) {
        init = get_constant(expression.list[2]);
        if (!init) init = eval(expression.list[2]);
    }
    return create_local_variable(expression.list[1].str, type, init);
}

llvm::GlobalVariable* Compiler::create_global_variable(const Expression& expression) {
    // global type name init
    assert(expression.list.size() == 4);
    llvm::Type* type = get_type(expression.list[1].str);
    return create_global_variable(expression.list[2].str, type, expression.list[3]);
}

llvm::StoreInst* Compiler::set_variable(const Expression& expression) {
    assert(expression.list.size() == 3);
    auto& name = expression.list[1].str;
    auto& exp = expression.list[2];
    llvm::Value* result = symbol_table.get(name).ptr;
    return builder.CreateStore(eval(exp), result);
}

llvm::Value* Compiler::eval_sum(const Expression& expression) {
    BINARY_OP(CreateAdd, expression, "temp_add", >=)
}

llvm::Value* Compiler::eval_sub(const Expression& expression) {
    BINARY_OP(CreateSub, expression, "temp_sub", >=)
}

llvm::Value* Compiler::eval_mul(const Expression& expression) {
    BINARY_OP(CreateMul, expression, "temp_mul", >=)
}

llvm::Value* Compiler::eval_div(const Expression& expression) {
    BINARY_OP(CreateSDiv, expression, "temp_div", >=)
}

llvm::Value* Compiler::eval_less_than(const Expression& expression) {
    BINARY_OP(CreateICmpSLT, expression, "temp_lt", ==)
}

llvm::Value* Compiler::eval_greater_than(const Expression& expression) {
    BINARY_OP(CreateICmpSGT, expression, "temp_gt", ==)
}

llvm::Value* Compiler::eval_less_than_eq(const Expression& expression) {
    BINARY_OP(CreateICmpSLE, expression, "temp_lte", ==)
}

llvm::Value* Compiler::eval_greater_than_eq(const Expression& expression) {
    BINARY_OP(CreateICmpSGE, expression, "temp_gte", ==)
}

llvm::Value* Compiler::eval_equal(const Expression& expression) {
    BINARY_OP(CreateICmpEQ, expression, "temp_eq", ==)
}

llvm::Value* Compiler::eval_not_equal(const Expression& expression) {
    BINARY_OP(CreateICmpNE, expression, "temp_neq", ==)
}

llvm::PHINode* Compiler::eval_if(const Expression& expression) {
    assert(expression.list.size() == 4);
    const Expression& cond_exp = expression.list[1];
    const Expression& then_exp = expression.list[2];
    const Expression& else_exp = expression.list[3];

    // If a counter is not used, LLVM will assign numbers incrementally (for example then1, else2, condition3)
    //  which can be confusing, especially in nested expressions.
    static int _counter = 0;
    int counter = _counter++;  // storing a local copy
    // You might delay the attachment of the blocks to the functions to reorder the blocks in a more readable way
    //  by doing the following: current_function->getBasicBlockList().push_back(block); I prefer this way.
    llvm::BasicBlock* then_block = create_basic_block("then" + std::to_string(counter), current_function);
    llvm::BasicBlock* else_block = create_basic_block("else" + std::to_string(counter), current_function);
    llvm::BasicBlock* end_block = create_basic_block("if_end" + std::to_string(counter), current_function);


    llvm::Value* cond_res = eval(cond_exp);
    builder.CreateCondBr(cond_res, then_block, else_block);

    builder.SetInsertPoint(then_block);
    llvm::Value* then_res = eval(then_exp);
    // Since the block may contain nested statements, the current then_block may be terminated early, and after the eval
    //  of the then_exp, we end up in a completely different block. This block is where the branching instruction to
    //  the end_block will be inserted, and this block is the block that would be a predecessor of the end_block, instead
    //  of the current then_block. Thus, we need to use the block we ended up upon after evaluating the then_exp instead
    //  of the current then_block (the two blocks may be the same) when referring to the phi instruction, which expects
    //  the provided blocks to be its predecessor.
    builder.CreateBr(end_block);
    then_block = builder.GetInsertBlock();

    builder.SetInsertPoint(else_block);
    llvm::Value* else_res = eval(else_exp);
    // Same as the then block.
    builder.CreateBr(end_block);
    else_block = builder.GetInsertBlock();

    assert(then_res->getType() == else_res->getType());
    builder.SetInsertPoint(end_block);

    llvm::PHINode* phi = builder.CreatePHI(then_res->getType(), 2, "if_result" + std::to_string(counter));
    phi->addIncoming(then_res, then_block);
    phi->addIncoming(else_res, else_block);

    return phi;
}

llvm::Value* Compiler::eval_while(const Expression& expression) {
    assert(expression.list.size() == 3);
    const Expression& cond_exp = expression.list[1];
    const Expression& body_exp = expression.list[2];

    // If a counter is not used, LLVM will assign numbers incrementally (for example while_cond1, while_body2, while_end3)
    //  which can be confusing, especially in nested expressions.
    static int _counter = 0;
    int counter = _counter++;  // storing a local copy
    // You might delay the attachment of the blocks to the functions to reorder the blocks in a more readable way
    //  by doing the following: current_function->getBasicBlockList().push_back(block); I prefer this way.
    llvm::BasicBlock* cond_block = create_basic_block("while_cond" + std::to_string(counter), current_function);
    llvm::BasicBlock* body_block = create_basic_block("while_body" + std::to_string(counter), current_function);
    llvm::BasicBlock* end_block = create_basic_block("while_end" + std::to_string(counter), current_function);

    // From the current block.
    builder.CreateBr(cond_block);

    builder.SetInsertPoint(cond_block);
    llvm::Value* cond_res = eval(cond_exp);
    builder.CreateCondBr(cond_res, body_block, end_block);

    builder.SetInsertPoint(body_block);
    eval(body_exp);
    builder.CreateBr(cond_block);

    builder.SetInsertPoint(end_block);

    return builder.getInt32(0);
}
