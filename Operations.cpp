#include "Compiler.h"

llvm::Value* Compiler::call_printf(const Expression& expression) {
    std::vector<llvm::Value*> args;
    for (int i = 1; i < expression.list.size(); i++) {
        args.push_back(eval(expression.list[i]));
    }
    return call_function("printf", args);
}

llvm::Value* Compiler::eval_scope(const Expression& expression) {
    for (int i = 1; i < expression.list.size() - 1; i++)
        eval(expression.list[i]);
    return eval(expression.list.back());
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
