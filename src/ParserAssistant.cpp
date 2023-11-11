#include "parsing/ParserAssistant.h"

TranslationUnitNode* ParserAssistant::construct_result()
{
    size_t n = stack.size();
    std::vector<ASTNode*> elements(n);
    for (int i = 0; i < n; i++)
        elements[n - i - 1] = stack.pop_node();
    return compiler->create_node<TranslationUnitNode>(std::move(elements));
}

void ParserAssistant::create_definition()
{
    auto value = stack.pop_node();
    auto name = stack.pop_str();
    auto type = stack.pop_type();
    stack.push(compiler->create_node<GlobalVariableDefinitionNode>(std::move(name), type, (ValueNode*)value));
}
