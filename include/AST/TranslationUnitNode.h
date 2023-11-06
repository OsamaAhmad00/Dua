#pragma once

#include <AST/ASTNode.h>

class TranslationUnitNode : public ASTNode
{
    std::vector<ASTNode*> elements;

public:

    void add(ASTNode* node) { elements.push_back(node); }

    NoneValue eval() override
    {
        for (auto element : elements)
            element->eval();
        return none_value();
    }
};