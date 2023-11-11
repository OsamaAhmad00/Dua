#pragma once

#include <vector>
#include <AST/ASTNode.h>
#include <types/TypeBase.h>

struct StackElement
{
    // TODO use a more efficient approach.
    enum { STR, NODE, TYPE } element_type;
    std::string str;
    ASTNode *node;
    TypeBase *type;
    StackElement(std::string str) : str(std::move(str)), element_type(STR) {}
    StackElement(ASTNode* node) : node(node), element_type(NODE) {}
    StackElement(TypeBase* type) : type(type), element_type(TYPE) {}
};

struct ParserAssistantStack
{
    std::vector<StackElement> stack;

    void push(std::string str) { stack.emplace_back(std::move(str)); }

    void push(ASTNode* node) { stack.emplace_back(node); }

    void push(TypeBase* type) { stack.emplace_back(type); }

    std::string pop_str() {
        assert(stack.back().element_type == StackElement::STR);
        std::string result = std::move(stack.back().str);
        stack.pop_back();
        return result;
    }

    ASTNode* pop_node() {
        assert(stack.back().element_type == StackElement::NODE);
        ASTNode* result = stack.back().node;
        stack.pop_back();
        return result;
    }

    TypeBase* pop_type() {
        assert(stack.back().element_type == StackElement::TYPE);
        TypeBase* result = stack.back().type;
        stack.pop_back();
        return result;
    }

    size_t size() { return stack.size(); }
};

