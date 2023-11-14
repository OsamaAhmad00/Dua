#include "parsing/ParserAssistant.h"

TranslationUnitNode* ParserAssistant::construct_result()
{
    size_t n = nodes_count();
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
    auto value = pop_node();
    auto name = pop_str();
    auto type = pop_type();
    if (is_in_global_scope()) {
        push_node<GlobalVariableDefinitionNode>(std::move(name), type, (ValueNode *) value);
    } else {
        push_node<LocalVariableDefinitionNode>(std::move(name), type, value);
    }
    inc_statements();
}

void ParserAssistant::create_function_declaration()
{
    inc_statements();
    FunctionNodeBase::FunctionSignature signature;
    signature.is_var_arg = is_var_arg;
    signature.params.resize(param_count);
    // The params are pushed in reverse-order
    for (int i = 0; i < param_count; i++)
        signature.params[param_count - i - 1] = {pop_str(), pop_type()};
    signature.return_type = pop_type();
    signature.name = pop_str();
    push_node<FunctionDefinitionNode>(signature, nullptr);
}

void ParserAssistant::create_block_statement()
{
    size_t n = statement_counters.back();
    std::vector<ASTNode*> statements(n);
    for (int i = 0; i < n; i++)
        statements[n - i - 1] = pop_node();
    push_node<BlockNode>(statements);

    leave_scope();
    inc_statements();
}

void ParserAssistant::create_function_definition()
{
    // We're merging the block and the
    //  declaration into a single node.
    dec_statements();

    ASTNode* body = pop_node();
    // The function is not popped, so it's still in the stack.
    auto function = (FunctionDefinitionNode*)nodes.back();
    function->set_body(body);
}

void ParserAssistant::create_if()
{
    size_t n = leave_conditional();

    std::vector<ASTNode*> conditions(n);
    std::vector<ASTNode*> branches(n);

    ASTNode* else_node = nullptr;
    if (has_else) {
        else_node = pop_node();
        dec_statements();
    }

    for (size_t i = 0; i < n; i++) {
        branches[n - i - 1] = pop_node();
        conditions[n - i - 1] = pop_node();
        dec_statements();
    }

    if (else_node) branches.push_back(else_node);

    push_node<IfNode>(std::move(conditions), std::move(branches));

    inc_statements();
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
    branches_count.push_back(0);
}

size_t ParserAssistant::leave_conditional() {
    size_t result = branches_count.back();
    branches_count.pop_back();
    return result;
}

void ParserAssistant::inc_statements() {
    statement_counters.back() += 1;
}

void ParserAssistant::dec_statements() {
    statement_counters.back() -= 1;
}

void ParserAssistant::inc_branches() {
    branches_count.back() += 1;
}

void ParserAssistant::dec_branches() {
    branches_count.back() -= 1;
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
    ASTNode* lhs = pop_node();
    push_node<AssignmentExpressionNode>(lhs, rhs);
}