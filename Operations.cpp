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
