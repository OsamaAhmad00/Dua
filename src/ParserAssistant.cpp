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
    if (scope_depth == 0)
        push_node<GlobalVariableDefinitionNode>(std::move(name), type, (ValueNode*)value);
    else
        push_node<LocalVariableDefinitionNode>(std::move(name), type, value);
}

void ParserAssistant::create_function_declaration()
{
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
    std::vector<ASTNode*> statements(statements_count);
    for (int i = 0; i < statements_count; i++)
        statements[statements_count - i - 1] = pop_node();
    push_node<BlockNode>(statements);
}

void ParserAssistant::create_function_definition()
{
    ASTNode* body = pop_node();
    // The function is not popped, so it's still in the stack.
    auto function = (FunctionDefinitionNode*)nodes.back();
    function->set_body(body);
}