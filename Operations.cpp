#include "Compiler.h"

#define BINARY_OP(OP, EXP, LABEL, PARAM_RESTRICTION) \
assert(EXP.list.size() PARAM_RESTRICTION 3); \
llvm::Value* result = builder->OP(eval(EXP.list[1]), eval(EXP.list[2]), LABEL);\
for (int i = 3; i < EXP.list.size(); i++) \
    result = builder->OP(result, eval(EXP.list[i]));                    \
return result;

llvm::Value* Compiler::call_printf(const Expression& expression) {
    std::vector<llvm::Value*> args;
    for (int i = 1; i < expression.list.size(); i++) {
        args.push_back(eval(expression.list[i]));
    }
    return call_function("printf", args);
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
    assert(expression.list.size() == 3);
    llvm::Constant* init = get_expression_value(expression.list[2]);
    return create_local_variable(expression.list[1].str, init);
}

llvm::GlobalVariable* Compiler::create_global_variable(const Expression& expression) {
    assert(expression.list.size() == 3);
    llvm::Constant* init = get_expression_value(expression.list[2]);
    return create_global_variable(expression.list[1].str, init);
}

llvm::Value* Compiler::set_variable(const Expression& expression) {
    assert(expression.list.size() == 3);
    auto& name = expression.list[1].str;
    auto& exp = expression.list[2];
    llvm::Value* result = symbol_table.contains(name) ?
            (llvm::Value*)symbol_table.get(name) : (llvm::Value*)symbol_table.get_global(name);
    builder->CreateStore(eval(exp), result);
    return result;
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
