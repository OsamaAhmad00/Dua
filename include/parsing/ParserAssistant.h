#pragma once

#include "AST/ASTNode.h"

#include "AST/TranslationUnitNode.h"

#include "AST/variable/GlobalVariableDefinitionNode.h"
#include "AST/variable/LocalVariableDefinitionNode.h"
#include "AST/AssignmentExpressionNode.h"
#include "AST/CompoundAssignmentExpressionNode.h"
#include "AST/ExpressionStatementNode.h"
#include "AST/BlockNode.h"
#include "AST/IfNode.h"
#include "AST/SequentialEvalNode.h"
#include "AST/SizeOfNode.h"
#include "AST/DeferredActionNode.h"
#include "AST/FreeNode.h"

#include "AST/class/ClassDefinitionNode.h"

#include "AST/loops/ForNode.h"
#include "AST/loops/WhileNode.h"
#include "AST/loops/DoWhileNode.h"
#include "AST/loops/ContinueNode.h"
#include "AST/loops/BreakNode.h"

#include "AST/function/FunctionCallNode.h"
#include "AST/function/FunctionDefinitionNode.h"
#include "AST/function/ReturnNode.h"

#include "AST/binary/BinaryExp.h"
#include "AST/binary/ComparisonNodes.h"
#include "AST/binary/ArithmeticNodes.h"
#include "AST/binary/BitManipulationNodes.h"

#include "AST/unary/NumericalUnaryExpressionNodes.h"
#include "AST/unary/CastExpressionNode.h"
#include "AST/unary/PostfixAdditionExpressionNode.h"

#include "AST/values/ValueNode.h"
#include "AST/values/ArrayValueNode.h"
#include "AST/values/FloatValueNodes.h"
#include "AST/values/IntegerValueNodes.h"
#include "AST/values/StringValueNode.h"

#include "AST/lvalue/LValueNode.h"
#include "AST/lvalue/VariableNode.h"
#include "AST/lvalue/ArrayIndexingNode.h"
#include "AST/lvalue/DereferenceNode.h"
#include "AST/lvalue/ClassFieldNode.h"
#include "AST/lvalue/LoadedLValueNode.h"
#include "AST/lvalue/MallocNode.h"

#include "types/IntegerTypes.h"
#include "types/FloatTypes.h"
#include "types/StringType.h"
#include "types/ArrayType.h"
#include "types/PointerType.h"
#include "types/ClassType.h"
#include "types/FunctionType.h"

#include "utils/TextManipulation.h"
#include "utils/ErrorReporting.h"


namespace dua
{

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
    std::vector<uint64_t> numbers;
    std::vector<ASTNode *> nodes;
    std::vector<Type *> types;

    // A stack for counting the number of statements
    //  inside the current scope, and for determining
    //  whether we're in a local or the global scope.
    // Has initially one counter for the global scope.
    std::vector<size_t> statement_counters { 0 };

    // Used to determine the number of branches in the
    //  current if/when statement/expression.
    std::vector<size_t> branch_counters;
    std::vector<bool> has_else;

    // Used to determine the number of parameters
    //  or arguments in function expressions.
    std::vector<size_t> argument_counters;
    std::vector<bool> var_arg_stack;

    std::vector<FieldConstructorArgs> fields_args;

    // Flags
    bool declared_malloc = false;
    bool declared_free = false;

    std::string pop_str() {
        auto result = std::move(strings.back());
        strings.pop_back();
        return result;
    }

    uint64_t pop_num() {
        auto result = numbers.back();
        numbers.pop_back();
        return result;
    }

    ASTNode *pop_node() {
        ASTNode *result = nodes.back();
        nodes.pop_back();
        return result;
    }

    template<typename T>
    T *pop_node_as() {
        auto result = pop_node();
        auto casted = dynamic_cast<T*>(result);
        if (casted == nullptr)
            report_internal_error("Unexpected node type");
        return casted;
    }

    Type *pop_type() {
        Type *result = types.back();
        types.pop_back();
        return result;
    }

public:

    void push_str(std::string str) {
        strings.push_back(std::move(str));
    }

    void push_num(uint64_t num) { numbers.push_back(num); }

    template<typename T, typename...Args>
    void push_node(Args...args) { nodes.push_back(compiler->create_node<T>(args...)); }

    template<typename T, typename...Args>
    void push_type(Args...args) { types.push_back(compiler->create_type<T>(args...)); }

    void push_null_node() { nodes.push_back(nullptr); }

    static int64_t get_i64(std::string num);
    static int32_t get_i32(std::string num);
    static int16_t get_i16(std::string num);
    static int8_t  get_i8 (std::string num);

    std::vector<ASTNode*> pop_args();

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
    void create_pointer_type();
    void create_array_type();
    void create_string_value();
    void create_dereference();
    void create_pre_inc();
    void create_pre_dec();
    void create_post_inc();
    void create_post_dec();
    void create_ternary_operator();
    void create_for();
    void create_empty_statement();
    void create_continue();
    void create_break();
    void create_do_while();
    void create_loaded_lvalue();
    void create_array_indexing();
    void create_logical_and();
    void create_logical_or();
    void create_class_type();
    void create_field_access();
    void create_constructor_call();
    void create_inferred_definition();
    void create_size_of_type();
    void create_size_of_expression();
    void create_type_of();
    void create_typename_type();
    void create_typename_expression();
    void create_function_type();
    void create_malloc();
    void create_free();
    void prepare_constructor();
    void prepare_destructor();
    void create_pointer_field_access();

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

    template <typename T>
    void create_compound_assignment() {
        auto rhs = pop_node();
        auto lhs = pop_node_as<LValueNode>();
        push_node<CompoundAssignmentExpressionNode<T>>(lhs, rhs);
    }

    void add_field_constructor_args();

    void enter_conditional();
    size_t leave_conditional();
    void inc_branches();
    void set_has_else();
    void create_if_statement();
    void create_if_expression();

    void enter_scope();
    size_t leave_scope();
    void inc_statements();
    void dec_statements();

    void enter_arg_list();
    size_t leave_arg_list();
    void inc_args();
    void push_var_arg(bool value);
    bool pop_var_arg();
    void create_function_call();

    void register_class();
    void finish_class_declaration();
    void start_class_definition();
    void create_class();

    bool is_in_global_scope();

    TranslationUnitNode* construct_result();

    void finish_parsing();
    void create_missing_methods();
    void create_empty_method_if_doesnt_exist(ClassType* cls, std::string&& name);
    void reset_symbol_table();

    void set_module_compiler(ModuleCompiler* compiler) { this->compiler = compiler; }
};

}
