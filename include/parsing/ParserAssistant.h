#pragma once

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


//  This class is used to make the semantic-actions in the parser grammar file
//  as minimal as possible, making the grammar clearer, and also the code more
//  isolated from the grammar, thus, more readable.
class ParserAssistant
{
    ModuleCompiler* compiler = nullptr;

    // These stacks are used to push every recent result into it.
    // If a node currently under construction requires previous
    //  results, it can pop this out of the appropriate stack and
    //  use it. When this node is constructed, it'll be pushed to
    //  the appropriate stack.
    // At the end, this stack will contain the elements of the
    //  translation unit. In other words, we're mimicking a stack
    //  machine, but with each datatype in a separate stack.
    std::vector<std::string> strings;
    std::vector<ASTNode *> nodes;
    std::vector<TypeBase *> types;

    // A stack for counting the number of statements
    //  inside the current scope.
    // Has initially one counter for the global scope.
    std::vector<size_t> statements_count { 0 };

    std::string pop_str() {
        auto result = std::move(strings.back());
        strings.pop_back();
        return result;
    }

    ASTNode *pop_node() {
        ASTNode *result = nodes.back();
        nodes.pop_back();
        return result;
    }

    TypeBase *pop_type() {
        TypeBase *result = types.back();
        types.pop_back();
        return result;
    }

public:

    // Used while parsing functions.
    // If nested functions are supported, we would
    //  need to use a stack of these values instead.
    bool is_var_arg = false;
    size_t param_count = 0;
    size_t statements_count = 0;

    // Used mainly to determine whether we're in global scope or not.
    size_t scope_depth = 0;

    // These stacks are used to push every recent result into it.
    // If a node currently under construction requires previous
    //  results, it can pop this out of the appropriate stack and
    //  use it. When this node is constructed, it'll be pushed to
    //  the appropriate stack.
    // At the end, this stack will contain the elements of the
    //  translation unit. In other words, we're mimicking a stack
    //  machine, but with each datatype in a separate stack.
    ADD_STACK_DATATYPE(str, std::string)
    ADD_TEMPLATED_STACK_DATATYPE(node, ASTNode*, compiler->create_node<T>);
    ADD_TEMPLATED_STACK_DATATYPE(type, TypeBase*, compiler->create_type<T>);

    void create_definition();
    void create_function_declaration();
    void create_block_statement();
    void create_function_definition();

    TranslationUnitNode* construct_result();

    void set_module_compiler(ModuleCompiler* compiler) { this->compiler = compiler; }
};