#pragma once

#include <parsing/ParserAssistantStack.h>

#include "AST/ASTNode.h"

#include "AST/TranslationUnitNode.h"

#include "AST/GlobalVariableDefinitionNode.h"
#include "AST/LocalVariableDefinitionNode.h"
#include "AST/AssignmentExpressionNode.h"
#include "AST/Block.h"
#include "AST/IfNode.h"
#include "AST/WhileNode.h"

#include "AST/function/FunctionNodeBase.h"
#include "AST/function/FunctionCallNode.h"
#include "AST/function/FunctionDefinitionNode.h"
#include "AST/function/ReturnNode.h"

#include "AST/binary/BinaryExp.h"
#include "AST/binary/ComparisonNodes.h"
#include "AST/binary/ArithmeticNodes.h"

#include "AST/unary/NumericalUnaryExpressionNodes.h"
#include "AST/unary/CastExpressionNode.h"

#include "AST/terminals/ValueNode.h"
#include "AST/terminals/ArrayValueNode.h"
#include "AST/terminals/FloatValueNodes.h"
#include "AST/terminals/IntegerValueNodes.h"
#include "AST/terminals/StringValueNode.h"
#include "AST/terminals/VariableNode.h"

#include "types/IntegerTypes.h"
#include "types/FloatTypes.h"
#include "types/StringType.h"
#include "types/ArrayType.h"


// This class is responsible for accepting the text (or some other intermediate form)
//  representation from the parser, and maintaining an internal state about the current
//  situation while parsing. This is to make the semantic-actions in the parser grammar
//  file as minimal as possible, making the grammar clearer, and also the code more
//  isolated from the grammar, thus, more readable.
class ParserAssistant
{
    ModuleCompiler* compiler = nullptr;

    // This stack is used to push every recent result into it.
    // If a node currently under construction requires previous
    //  results, it can pop this out of this stack and use it.
    //  When this node is constructed, it'll be pushed here.
    // At the end, this stack will contain the elements of
    //  the translation unit.
    ParserAssistantStack stack;

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