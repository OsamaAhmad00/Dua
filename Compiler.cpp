#include "Compiler.h"
#include "Parser.h"

Compiler::Compiler(const std::string& name) :
    context(std::make_unique<llvm::LLVMContext>()),
    module(std::make_unique<llvm::Module>(name, *context)),
    builder(std::make_unique<llvm::IRBuilder<>>(*context)),
    temp_builder(std::make_unique<llvm::IRBuilder<>>(*context)),
    parser(new syntax::Parser)
{
    init_external_references();
    init_primitive_types();
}

Compiler::~Compiler() { delete parser; }

void Compiler::compile(const std::string& code, const std::string& outfile) {

    // Parse
    // Since there might be more than one function, put all of
    //  them in a list, and parse them one after the other.
    Expression expression = parser->parse("(" + code + ")");

    // Generate IR
    for (auto& exp : expression.list)
        eval(exp);

    // Output
    module->print(llvm::outs(), nullptr);  // Print
    save_module(outfile);
}

void Compiler::init_primitive_types() {
    types["int"] = builder->getInt32Ty();
    types["str"] = builder->getInt8PtrTy();
    types["void"] = nullptr;
}

llvm::GlobalVariable* Compiler::create_global_variable(const std::string& name, llvm::Type* type, const Expression& init_exp)
{
    module->getOrInsertGlobal(name, type);
    llvm::GlobalVariable* variable = module->getGlobalVariable(name);
    variable->setConstant(false);
    variable->setAlignment(llvm::MaybeAlign(4));
    variable->setExternallyInitialized(false);
    if (llvm::Constant *value = get_constant(init_exp); value)
        variable->setInitializer(value);
    else
        throw std::runtime_error("Initialization of a global variable with non-constant value");
    symbol_table.insert_global(name, variable);
    return variable;
}

llvm::AllocaInst* Compiler::create_local_variable(const std::string& name, llvm::Type* type, llvm::Value* init)
{
    llvm::BasicBlock* entry = &current_function->getEntryBlock();
    temp_builder->SetInsertPoint(entry, entry->begin());
    llvm::AllocaInst* variable = temp_builder->CreateAlloca(type, 0, name);
    if (init) {
        builder->CreateStore(init, variable);
    }
    symbol_table.insert(name, variable);
    return variable;
}

llvm::LoadInst* Compiler::get_local_variable(const std::string& name)
{
    llvm::AllocaInst* variable = symbol_table.get(name);
    return builder->CreateLoad(variable->getAllocatedType(), variable);
}

llvm::LoadInst* Compiler::get_global_variable(const std::string& name)
{
    llvm::GlobalVariable* variable = symbol_table.get_global(name);
    return builder->CreateLoad(variable->getValueType(), variable);
}

llvm::Value* Compiler::eval(const Expression& expression) {
    switch (expression.type)
    {
        case SExpressionType::SYMBOL:
            if (expression.str == "true" || expression.str == "false") {
                return builder->getInt1(expression.str == "true");
            } else {
                if (symbol_table.contains(expression.str))
                    return get_local_variable(expression.str);
                else if (symbol_table.contains_global(expression.str))
                    return get_global_variable(expression.str);
                else
                    throw std::runtime_error("Unknown identifier");
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
                else if (first.str == "global")
                    return create_global_variable(expression);
                else if (get_type(first.str, false) != nullptr)
                    return create_local_variable(expression);
                else if (first.str == "set")
                    return set_variable(expression);
                else if (first.str == "+")
                    return eval_sum(expression);
                else if (first.str == "-")
                    return eval_sub(expression);
                else if (first.str == "*")
                    return eval_mul(expression);
                else if (first.str == "/")
                    return eval_div(expression);
                else if (first.str == "<")
                    return eval_less_than(expression);
                else if (first.str == ">")
                    return eval_greater_than(expression);
                else if (first.str == "<=")
                    return eval_less_than_eq(expression);
                else if (first.str == ">=")
                    return eval_greater_than_eq(expression);
                else if (first.str == "==")
                    return eval_equal(expression);
                else if (first.str == "!=")
                    return eval_not_equal(expression);
                else if (first.str == "if")
                    return eval_if(expression);
                else if (first.str == "while")
                    return eval_while(expression);
                else if (first.str == "fun" || first.str == "varfun")
                    return eval_function(expression);
                else if (first.str == "return")
                    return eval_return(expression);
                else
                    return eval_function_call(expression);
            }
            return eval(expression.list.back());
    }
    return nullptr;
}

llvm::Type* Compiler::get_type(const std::string& str, bool panic_if_invalid) {
    auto result = types.find(str);
    if (result == types.end()) {
        if (panic_if_invalid)
            throw std::runtime_error("No such data type");
        else
            return nullptr;
    }
    return result->second;
}

llvm::Constant* Compiler::get_constant(const Expression& expression) {
    switch (expression.type) {
        case SExpressionType::NUMBER:
            return builder->getInt32(expression.num);
        case SExpressionType::STRING:
            return builder->CreateGlobalStringPtr(expression.str);
        default:  // Symbol or List
            return nullptr;
    }
}

llvm::ConstantInt* Compiler::create_integer_literal(long long num) {
    return builder->getInt32(num);
}

llvm::Constant* Compiler::create_string_literal(const std::string& name, const std::string& str) {
    return builder->CreateGlobalStringPtr(str, name);
}

llvm::CallInst* Compiler::call_function(const std::string& name, const std::vector<llvm::Value*>& args) {
    llvm::Function* function = module->getFunction(name);
    return builder->CreateCall(function, args);
}

llvm::Function* Compiler::define_function(const std::string& name, const Expression& body, const std::string& return_type, const Parameters& parameters, bool is_var_arg) {
    llvm::Function* function = module->getFunction(name);

    if (!function)
        function = declare_function(name, return_type, parameters, is_var_arg);

    llvm::Function* old_function = current_function;
    llvm::BasicBlock* old_block = builder->GetInsertBlock();
    create_basic_block("entry", function);
    builder->SetInsertPoint(&function->back());
    current_function = function;

    symbol_table.push_scope();

    int i = 0;
    for (llvm::Argument& arg : function->args()) {
        auto& param = parameters[i++];
        arg.setName(param.second);
        create_local_variable(param.second, get_type(param.first), &arg);
    }

    eval(body);

    symbol_table.pop_scope();

    builder->SetInsertPoint(old_block);
    current_function = old_function;

    return function;
}

llvm::Function* Compiler::declare_function(const std::string& name, const std::string& return_type, const Parameters& parameters, bool is_var_arg) {
    llvm::Type* ret = get_type(return_type);
    std::vector<llvm::Type*> parameter_types;
    for (auto& param: parameters)
        parameter_types.push_back(get_type(param.first));
    llvm::FunctionType* type = llvm::FunctionType::get(ret, parameter_types, is_var_arg);
    llvm::Function* function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, name, *module);
    llvm::verifyFunction(*function);
    return function;
}

llvm::BasicBlock* Compiler::create_basic_block(const std::string& name, llvm::Function* function) {
    return llvm::BasicBlock::Create(*context, name, function);
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