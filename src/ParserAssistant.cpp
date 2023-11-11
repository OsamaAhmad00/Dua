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
    push_node<GlobalVariableDefinitionNode>(std::move(name), type, (ValueNode*)value);
}
