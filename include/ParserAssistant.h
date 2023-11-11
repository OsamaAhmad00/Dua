#pragma once

#include <AST/ASTNode.h>

#include <AST/TranslationUnitNode.h>

#include <AST/GlobalVariableDefinitionNode.h>
#include <AST/LocalVariableDefinitionNode.h>
#include <AST/AssignmentExpressionNode.h>
#include <AST/Block.h>
#include <AST/IfNode.h>
#include <AST/WhileNode.h>

#include <AST/function/FunctionNodeBase.h>
#include <AST/function/FunctionCallNode.h>
#include <AST/function/FunctionDefinitionNode.h>
#include <AST/function/ReturnNode.h>

#include <AST/binary/BinaryExp.h>
#include <AST/binary/ComparisonNodes.h>
#include <AST/binary/ArithmeticNodes.h>

#include <AST/unary/NumericalUnaryExpressionNodes.h>
#include <AST/unary/CastExpressionNode.h>

#include <AST/terminals/ValueNode.h>
#include <AST/terminals/ArrayValueNode.h>
#include <AST/terminals/FloatValueNodes.h>
#include <AST/terminals/IntegerValueNodes.h>
#include <AST/terminals/StringValueNode.h>
#include <AST/terminals/VariableNode.h>

#include <types/IntegerTypes.h>
#include <types/FloatTypes.h>
#include <types/StringType.h>
#include <types/ArrayType.h>


// This class is responsible for accepting the text (or some other intermediate form)
//  representation from the parser, and maintaining an internal state about the current
//  situation while parsing. This is to make the semantic-actions in the parser grammar
//  file as minimal as possible, making the grammar clearer, and also the code more
//  isolated from the grammar, thus, more readable.
class ParserAssistant
{
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

    struct Stack
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

    ModuleCompiler* compiler = nullptr;

    // This stack is used to push every recent result into it.
    // If a node currently under construction requires previous
    //  results, it can pop this out of this stack and use it.
    //  When this node is constructed, it'll be pushed here.
    // At the end, this stack will contain the elements of
    //  the translation unit.
    Stack stack;

public:

    void push_str(std::string str) {
        stack.push(std::move(str));
    }

    template<typename T, typename ...Args>
    void push_type(Args... args) {
        stack.push(compiler->create_type<T>(args...));
    }

    template<typename T, typename ...Args>
    void push_node(Args... args) {
        stack.push(compiler->create_node<T>(args...));
    }

    void create_definition();

    TranslationUnitNode* construct_result();

    void set_module_compiler(ModuleCompiler* compiler) { this->compiler = compiler; }
};