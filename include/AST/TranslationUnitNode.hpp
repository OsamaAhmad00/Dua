#pragma once

#include <AST/ASTNode.hpp>
#include <types/VoidType.hpp>
#include "AST/variable/GlobalVariableDefinitionNode.hpp"

namespace dua
{

class TranslationUnitNode : public ASTNode
{
    std::vector<ASTNode*> elements;

public:

    explicit TranslationUnitNode(ModuleCompiler* compiler) { this->compiler = compiler; }

    TranslationUnitNode(ModuleCompiler* compiler, std::vector<ASTNode*> elements)
        : elements(std::move(elements)) { this->compiler = compiler; }

    void add(ASTNode* node) { elements.push_back(node); }

    NoneValue eval() override
    {
        for (auto element : elements)
            if (element->as<GlobalVariableDefinitionNode>() == nullptr)
                element->eval();
        return none_value();
    }
};

}
