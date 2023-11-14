#pragma once

#include "AST/ASTNode.h"

#include "AST/TranslationUnitNode.h"

#include "AST/GlobalVariableDefinitionNode.h"
#include "AST/LocalVariableDefinitionNode.h"
#include "AST/AssignmentExpressionNode.h"
#include "AST/ExpressionStatementNode.h"
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
#include "types/PointerType.h"


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
    //  inside the current scope, and for determining
    //  whether we're in a local or the global scope.
    // Has initially one counter for the global scope.
    std::vector<size_t> statement_counters { 0 };

    // Used to determine the number of branches in the
    //  current if/when statement/expression.
    std::vector<size_t> branch_counters;

    // Used to determine the number of arguments
    //  in a function call expression.
    std::vector<size_t> argument_counters;

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

    // Used when constructing if statements
    bool has_else = false;

    void push_str(std::string str) { strings.push_back(std::move(str)); }
    size_t str_count() { return strings.size(); }

    template<typename T, typename...Args>
    void push_node(Args...args) {
        nodes.push_back(compiler->create_node<T>(args...));
    }
    size_t nodes_count() { return nodes.size(); }

    template<typename T, typename...Args>
    void push_type(Args...args) { types.push_back(compiler->create_type<T>(args...)); }
    size_t types_count() { return types.size(); }

    void create_variable_declaration();
    void create_variable_definition();
    void create_function_declaration();
    void create_block();
    void create_function_definition_block_body();
    void create_function_definition_expression_body();
    void create_expression_statement();
    void create_return();
    void create_while();
    void create_assignment();
    void create_cast();
    void create_address_of();
    void create_pointer_type();

    template<typename T>
    void create_unary_expr() {
        push_node<T>(pop_node());
    }

    template<typename T>
    void create_binary_expr() {
        auto rhs = pop_node();
        auto lhs = pop_node();
        push_node<T>(lhs, rhs);
    }

    void enter_conditional();
    size_t leave_conditional();
    void inc_branches();
    void create_if_statement();
    void create_if_expression();

    void enter_scope();
    size_t leave_scope();
    void inc_statements();
    void dec_statements();

    void enter_fun_call();
    size_t leave_fun_call();
    void inc_args();
    void create_function_call();

    bool is_in_global_scope();

    TranslationUnitNode* construct_result();

    void set_module_compiler(ModuleCompiler* compiler) { this->compiler = compiler; }
};