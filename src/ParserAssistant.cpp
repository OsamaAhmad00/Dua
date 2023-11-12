#include "parsing/ParserAssistant.h"

TranslationUnitNode* ParserAssistant::construct_result()
{
    size_t n = nodes_count();
    std::vector<ASTNode*> elements(n);
    for (int i = 0; i < n; i++)
        elements[n - i - 1] = pop_node();
    return compiler->create_node<TranslationUnitNode>(std::move(elements));
}

void ParserAssistant::create_definition()
{
    auto value = pop_node();
    auto name = pop_str();
    auto type = pop_type();
    // This stack contains as many counters as the
    //  current level of nesting.
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
    size_t n = statements_count.back();
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

    auto branch = pop_node();
    auto condition = pop_node();

    push_node<IfNode>(condition, branch, nullptr);

    // For if conditionals, you need to have the top node as a result,
    //  but you need to keep track of the next insertion point as well.
    //  For this, we push the top node twice at the beginning, once as
    //  the final result, and the second as the next insertion point.
    //  Upon insertions of further branches, the insertion point is
    //  popped and the new insertion point is pushed. Upon finishing
    //  construction of the conditional, the insertion point is popped,
    //  and the first node is kept as the result of the operation.
    //  This additional node doesn't increment the number of statements.
    nodes.push_back(nodes.back());
}

void ParserAssistant::add_if_branch()
{
    auto branch = pop_node();
    auto condition = pop_node();

    auto node = (IfNode*)pop_node();
    nodes.push_back(&node->add_branch_at_bottom(condition, branch));

    dec_statements();
}

void ParserAssistant::set_else_branch()
{
    auto branch = pop_node();

    auto node = (IfNode*)pop_node();
    // No insertion point is pushed here.
    node->set_else(branch);

    dec_statements();
}

void ParserAssistant::set_no_else() { pop_node(); }

void ParserAssistant::enter_scope() {
    statements_count.push_back(0);
}

void ParserAssistant::leave_scope() {
    statements_count.pop_back();
}

void ParserAssistant::inc_statements() {
    statements_count.back() += 1;
}

void ParserAssistant::dec_statements() {
    statements_count.back() -= 1;
}

bool ParserAssistant::is_in_global_scope() {
    return statements_count.size() == 1;
}

void ParserAssistant::create_expression_statement() {
    push_node<ExpressionStatementNode>(pop_node());
    // It takes an expression that didn't count as a
    //  statement, and turn it into a statement.
    inc_statements();
}
