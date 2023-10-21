#include "Compiler.h"
#include "Parser.h"

Compiler::Compiler(const std::string& name) {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>(name, *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);

    parser = new syntax::Parser;

    init_external_references();
}

Compiler::~Compiler() { delete parser; }

void Compiler::compile(const std::string& code, const std::string& outfile) {

    // Parse
    Expression expression = parser->parse(code);

    // Generate IR
    eval_main(expression);

    // Output
    module->print(llvm::outs(), nullptr);  // Print
    save_module(outfile);
}

llvm::GlobalVariable* Compiler::create_global_variable(const std::string& name, llvm::Constant* initializer)
{
    module->getOrInsertGlobal(name, initializer->getType());
    llvm::GlobalVariable* variable = module->getGlobalVariable(name);
    variable->setConstant(false);
    variable->setAlignment(llvm::MaybeAlign(4));
    variable->setInitializer(initializer);
    return variable;
}

llvm::Constant* Compiler::get_global_variable(const std::string& name)
{
    return module->getGlobalVariable(name)->getInitializer();
}

llvm::Value* Compiler::eval(const Expression& expression) {
    switch (expression.type)
    {
        case SExpressionType::SYMBOL:
            if (expression.str == "true" || expression.str == "false") {
                return builder->getInt1(expression.str == "true");
            } else {
                return get_global_variable(expression.str);
            }
        case SExpressionType::STRING:
            static int str_lit_cnt = 0;
            return create_string_literal("literal_" + std::to_string(str_lit_cnt++), expression.str);
        case SExpressionType::NUMBER:
            return create_integer_literal(expression.num);
        case SExpressionType::LIST:
            const Expression& first = expression.list.front();
            if (first.type == SExpressionType::SYMBOL) {
                if (first.str == "printf")
                    return call_printf(expression);
                else if (first.str == "scope")
                    return eval_scope(expression);
                else if (first.str == "var")
                    return create_variable(expression);
            }
    }
}

llvm::ConstantInt* Compiler::create_integer_literal(long long num) {
    return builder->getInt64(num);
}

llvm::Constant* Compiler::create_string_literal(const std::string& name, const std::string& str) {
    return builder->CreateGlobalStringPtr(str, name);
}

llvm::CallInst* Compiler::call_function(const std::string& name, const std::vector<llvm::Value*>& args) {
    llvm::Function* function = module->getFunction(name);
    return builder->CreateCall(function, args);
}

void Compiler::eval_main(Expression& expression) {
    llvm::Function* function = create_function("main");
    construct_function_body(function, expression);
}

void Compiler::construct_function_body(llvm::Function* function, Expression& body) {

    llvm::IRBuilderBase::InsertPoint old_insert_point = builder->saveIP();
    builder->SetInsertPoint(&function->back());

    eval(body);

    builder->CreateRet(builder->getInt32(0));

    builder->restoreIP(old_insert_point);
}

llvm::Function* Compiler::create_function(const std::string& name) {
    llvm::Function* function = module->getFunction(name);

    if (!function) {
        llvm::FunctionType* type = llvm::FunctionType::get(builder->getInt32Ty(), false);
        function = create_function_prototype(name, type);
    }

    attach_function_entry_block(function);

    return function;
}

llvm::Function* Compiler::create_function_prototype(const std::string& name, llvm::FunctionType* type) {
    llvm::Function* function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, *module);
    llvm::verifyFunction(*function);
    return function;
}

llvm::BasicBlock* Compiler::attach_function_entry_block(llvm::Function* function) {
    return llvm::BasicBlock::Create(*context, "entry", function);
}

void Compiler::save_module(const std::string& outfile) {
    std::error_code error;
    llvm::raw_fd_ostream out(outfile, error);
    module->print(out, nullptr);
}

void Compiler::init_external_references() {
    // i32 printf(i8*, ...)
    llvm::FunctionType* type = llvm::FunctionType::get(builder->getInt32Ty(), {builder->getInt8PtrTy()}, true);
    module->getOrInsertFunction("printf", type);
}