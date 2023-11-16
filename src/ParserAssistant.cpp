#include "parsing/ParserAssistant.h"

namespace dua
{

TranslationUnitNode* ParserAssistant::construct_result()
{
    size_t n = nodes.size();
    std::vector<ASTNode*> elements(n);
    for (int i = 0; i < n; i++)
        elements[n - i - 1] = pop_node();
    return compiler->create_node<TranslationUnitNode>(std::move(elements));
}

void ParserAssistant::create_variable_declaration()
{
    auto name = pop_str();
    auto type = pop_type();
    if (is_in_global_scope()) {
        throw std::runtime_error("Global variables must be initialized");
    } else {
        push_node<LocalVariableDefinitionNode>(std::move(name), type, nullptr);
    }
    inc_statements();
}

void ParserAssistant::create_variable_definition()
{
    auto value = pop_node_as<ValueNode>();
    auto name = pop_str();
    auto type = pop_type();
    if (is_in_global_scope()) {
        push_node<GlobalVariableDefinitionNode>(std::move(name), type, value);
    } else {
        push_node<LocalVariableDefinitionNode>(std::move(name), type, value);
    }
    inc_statements();
}

void ParserAssistant::create_function_declaration()
{
    if (!is_in_global_scope())
        throw std::runtime_error("Function declarations/definitions not allowed in a local scope");
    inc_statements();
    FunctionSignature signature;
    signature.is_var_arg = is_var_arg;
    signature.params.resize(param_count);
    // The params are pushed in reverse-order
    for (int i = 0; i < param_count; i++)
        signature.params[param_count - i - 1] = {pop_str(), pop_type()};
    signature.return_type = pop_type();
    auto name = pop_str();
    compiler->register_function(name, std::move(signature));
    push_node<FunctionDefinitionNode>(std::move(name), nullptr);
}

void ParserAssistant::create_block()
{
    size_t n = leave_scope();
    std::vector<ASTNode*> statements(n);
    for (int i = 0; i < n; i++)
        statements[n - i - 1] = pop_node();
    push_node<BlockNode>(statements);
}

void ParserAssistant::create_function_definition_block_body()
{
    ASTNode* body = pop_node();
    // The function is not popped, so it's still in the stack.
    auto function = pop_node_as<FunctionDefinitionNode>();
    function->set_body(body);
    nodes.push_back(function);
}

void ParserAssistant::create_function_definition_expression_body()
{
    ASTNode* body = compiler->create_node<ReturnNode>(pop_node());
    // The function is not popped, so it's still in the stack.
    auto function = pop_node_as<FunctionDefinitionNode>();
    function->set_body(body);
    nodes.push_back(function);
}

void ParserAssistant::create_if_statement()
{
    bool create_else = has_else.back();
    size_t n = leave_conditional();

    std::vector<ASTNode*> conditions(n);
    std::vector<ASTNode*> branches(n);

    ASTNode* else_node = nullptr;
    if (create_else) {
        else_node = pop_node();
        dec_statements();
    }

    for (size_t i = 0; i < n; i++) {
        branches[n - i - 1] = pop_node();
        conditions[n - i - 1] = pop_node();
        dec_statements();
    }

    if (else_node) branches.push_back(else_node);

    push_node<IfNode>(std::move(conditions), std::move(branches), false);

    inc_statements();
}

void ParserAssistant::create_if_expression()
{
    bool create_else = has_else.back();
    size_t n = leave_conditional();

    std::vector<ASTNode*> conditions(n);
    std::vector<ASTNode*> branches(n);

    ASTNode* else_node = nullptr;
    if (create_else) {
        else_node = pop_node();
    }

    for (size_t i = 0; i < n; i++) {
        branches[n - i - 1] = pop_node();
        conditions[n - i - 1] = pop_node();
    }

    if (else_node) branches.push_back(else_node);

    push_node<IfNode>(std::move(conditions), std::move(branches), true, pop_str());
}


void ParserAssistant::enter_scope() {
    statement_counters.push_back(0);
}

size_t ParserAssistant::leave_scope() {
    size_t result = statement_counters.back();
    statement_counters.pop_back();
    return result;
}

void ParserAssistant::enter_conditional() {
    branch_counters.push_back(0);
    has_else.push_back(false);
}

size_t ParserAssistant::leave_conditional() {
    size_t result = branch_counters.back();
    branch_counters.pop_back();
    has_else.pop_back();
    return result;
}

void ParserAssistant::inc_statements() {
    statement_counters.back() += 1;
}

void ParserAssistant::dec_statements() {
    statement_counters.back() -= 1;
}

void ParserAssistant::inc_branches() {
    branch_counters.back() += 1;
}

void ParserAssistant::set_has_else() {
    has_else.back() = true;
}

bool ParserAssistant::is_in_global_scope() {
    return statement_counters.size() == 1;
}

void ParserAssistant::create_expression_statement() {
    push_node<ExpressionStatementNode>(pop_node());
    // It takes an expression that didn't count as a
    //  statement, and turn it into a statement.
    inc_statements();
}

void ParserAssistant::create_return() {
    push_node<ReturnNode>(pop_node());
    // It takes an expression that didn't count as a
    //  statement, and turn it into a statement.
    inc_statements();
}

void ParserAssistant::create_while()
{
    ASTNode* body = pop_node();
    ASTNode* condition = pop_node();
    push_node<WhileNode>(condition, body);
}

void ParserAssistant::create_assignment()
{
    // We don't increment the statements counter here since this
    //  is an expression, and the expression statement would increase
    //  it as appropriate
    ASTNode* rhs = pop_node();
    auto lhs = pop_node_as<LValueNode>();
    push_node<AssignmentExpressionNode>(lhs, rhs);
}

void ParserAssistant::enter_fun_call() {
    argument_counters.push_back(0);
}

size_t ParserAssistant::leave_fun_call() {
    size_t result = argument_counters.back();
    argument_counters.pop_back();
    return result;
}

void ParserAssistant::inc_args() {
    argument_counters.back() += 1;
}

void ParserAssistant::create_function_call()
{
    std::string name = pop_str();
    size_t n = leave_fun_call();
    std::vector<ASTNode*> args(n);
    for (size_t i = 0; i < n; i++)
        args[n - i - 1] = pop_node();
    push_node<FunctionCallNode>(std::move(name), std::move(args));
}

void ParserAssistant::create_cast()
{
    auto type = pop_type();
    auto expr = pop_node();
    push_node<CastExpressionNode>(expr, type);
}

void ParserAssistant::create_pointer_type() {
    push_type<PointerType>(pop_type());
}

void ParserAssistant::create_array_type() {
    auto type = pop_type();
    auto size = pop_num();
    push_type<ArrayType>(type, size);
}

void ParserAssistant::create_string_value() {
    auto str = pop_str();
    push_node<StringValueNode>(str.substr(1, str.size() - 2));
}

void ParserAssistant::create_address_expr() {
    push_node<AddressNode>(pop_node(), false);
}

void ParserAssistant::create_dereference() {
    push_node<AddressNode>(pop_node(), true);
}

void ParserAssistant::create_pre_inc() {
    push_node<CompoundAssignmentExpressionNode<AdditionNode>>(
        pop_node_as<LValueNode>(),
        compiler->create_node<I32ValueNode>(1)
    );
}

void ParserAssistant::create_pre_dec() {
    push_node<CompoundAssignmentExpressionNode<SubtractionNode>>(
        pop_node_as<LValueNode>(),
        compiler->create_node<I32ValueNode>(1)
    );
}

void ParserAssistant::create_post_inc() {
    push_node<PostfixAdditionExpressionNode>(pop_node_as<LValueNode>(), 1);
}

void ParserAssistant::create_post_dec() {
    push_node<PostfixAdditionExpressionNode>(pop_node_as<LValueNode>(), -1);
}

void ParserAssistant::create_ternary_operator() {
    enter_conditional();
    inc_branches();
    set_has_else();
    create_if_expression();
}

void ParserAssistant::create_for()
{

    auto update = pop_node();
    auto body = pop_node();
    auto condition = pop_node();

    // Decrement for popping the body statement
    dec_statements();

    auto n = leave_scope();
    std::vector<ASTNode*> initializations(n);
    for (int i = 0; i < n; i++)
        initializations[n - i - 1] = pop_node();

    inc_statements();

    push_node<ForNode>(std::move(initializations), condition, update, body);
}

void ParserAssistant::create_empty_statement() {
    push_node<I8ValueNode>(0);
    inc_statements();
}

void ParserAssistant::create_continue() {
    push_node<ContinueNode>();
    inc_statements();
}

void ParserAssistant::create_break() {
    push_node<BreakNode>();
    inc_statements();
}

}
