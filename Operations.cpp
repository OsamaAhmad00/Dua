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

llvm::Value* Compiler::create_variable(const Expression& expression) {
    assert(expression.list.size() == 3);
    llvm::Value* init;
    switch (expression.list[2].type) {
        case SExpressionType::NUMBER:
            init = builder->getInt64(expression.list[2].num);
            break;
        case SExpressionType::STRING:
            return create_string_literal(expression.list[1].str, expression.list[2].str);
            break;
        default:
            init = eval(expression.list[2]);
    }

    auto x = llvm::dyn_cast<llvm::Constant>(init);
    return create_global_variable(expression.list[1].str, x);
}
