#pragma once

#include "AST/ASTNode.hpp"

#include "AST/TranslationUnitNode.hpp"

#include "AST/AssignmentExpressionNode.hpp"
#include "AST/CompoundAssignmentExpressionNode.hpp"
#include "AST/ExpressionStatementNode.hpp"
#include "AST/BlockNode.hpp"
#include "AST/IfNode.hpp"
#include "AST/SequentialEvalNode.hpp"
#include "AST/DeferredActionNode.hpp"
#include "AST/FreeNode.hpp"

#include "AST/class/ClassDefinitionNode.hpp"

#include "AST/variable/GlobalVariableDefinitionNode.hpp"
#include "AST/variable/LocalVariableDefinitionNode.hpp"
#include "AST/variable/ClassFieldDefinitionNode.hpp"

#include "AST/loops/ForNode.hpp"
#include "AST/loops/WhileNode.hpp"
#include "AST/loops/DoWhileNode.hpp"
#include "AST/loops/ContinueNode.hpp"
#include "AST/loops/BreakNode.hpp"

#include "AST/function/ExprFunctionCallNode.hpp"
#include "AST/function/FunctionCallNode.hpp"
#include "AST/function/MethodCallNode.hpp"
#include "AST/function/FunctionDefinitionNode.hpp"
#include "AST/function/ReturnNode.hpp"

#include "AST/binary/BinaryExp.hpp"
#include "AST/binary/ComparisonNodes.hpp"
#include "AST/binary/ArithmeticNodes.hpp"
#include "AST/binary/BitManipulationNodes.hpp"

#include "AST/unary/NumericalUnaryExpressionNodes.hpp"
#include "AST/unary/CastExpressionNode.hpp"
#include "AST/unary/PostfixAdditionExpressionNode.hpp"
#include "AST/unary/PrefixAdditionExpressionNode.hpp"

#include "AST/values/ValueNode.hpp"
#include "AST/values/ArrayValueNode.hpp"
#include "AST/values/FloatValueNodes.hpp"
#include "AST/values/IntegerValueNodes.hpp"
#include "AST/values/StringValueNode.hpp"

#include "AST/operators/TypeNameNode.hpp"
#include "AST/operators/SizeOfNode.hpp"
#include "AST/operators/IsTypeNode.hpp"

#include "AST/lvalue/LValueNode.hpp"
#include "AST/lvalue/VariableNode.hpp"
#include "AST/IndexingNode.hpp"
#include "AST/lvalue/DereferenceNode.hpp"
#include "AST/lvalue/ClassFieldNode.hpp"
#include "AST/lvalue/LoadedLValueNode.hpp"
#include "AST/lvalue/MallocNode.hpp"

#include "AST/types/TypeAliasNode.hpp"

#include "types/IntegerTypes.hpp"
#include "types/FloatTypes.hpp"
#include "types/ArrayType.hpp"
#include "types/PointerType.hpp"
#include "types/ClassType.hpp"
#include "types/FunctionType.hpp"
#include "types/ReferenceType.hpp"
#include "types/IdentifierType.hpp"
#include "types/TypeOfType.hpp"
#include "types/NoRefType.hpp"

#include "utils/TextManipulation.hpp"
#include "utils/ErrorReporting.hpp"

#include <set>

namespace dua
{

struct DeferredFieldArgs
{
    FunctionDefinitionNode* node;
    std::vector<FieldConstructorArgs> args;
    std::string owner_class;
    bool in_templated_class = false;
};

struct DeferredFunctionDeclaration
{
    FunctionDefinitionNode* node = nullptr;
    FunctionInfo info;
};

struct DeferredTemplatedClassDefinition
{
    std::string name;
    std::vector<const Type*> template_args;

    bool operator<(const DeferredTemplatedClassDefinition& other) const {
        return name < other.name || (name == other.name && template_args < other.template_args);
    }
};

struct ClassInfo
{
    // This is a struct that contains information used to define both
    //  templated and non-templated classes. This is used to define
    //  classes in a topological order in terms of inheritance
    bool is_templated;
    std::string name;
    ClassDefinitionNode* node;
    std::vector<std::string> children;
    std::vector<const Type*> template_args;
    ParentClassInfo parent;
};

//  This class is used to make the semantic-actions in the parser grammar file
//  as minimal as possible, making the grammar clearer, and also the code more
//  isolated from the grammar, thus, more readable.
class ParserAssistant
{
    friend class TemplatedNameResolver;

    ModuleCompiler* compiler = nullptr;

    std::string current_class;
    bool in_templated_class = false;

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
    std::vector<ASTNode*> nodes;
    std::vector<const Type*> types;

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
    std::vector<size_t> general_counters;
    std::vector<bool> var_arg_stack;

    // A deferred definitions that will happen after the parsing is done
    // This is to avoid constraining the order of definitions, and the
    //  necessity of declaring a class before using it in the same file.
    std::vector<ClassDefinitionNode*> class_definitions;
    std::set<DeferredTemplatedClassDefinition> templated_class_definitions;
    std::vector<DeferredFunctionDeclaration> function_definitions;
    std::vector<DeferredFieldArgs> constructors_field_args;

    std::vector<FieldConstructorArgs> fields_args;

    // Used to define the body of classes in a topological order, so that
    //  a definition of a class can just copy the fields of the direct
    //  parent only, without the need to copy all fields of all ancestors
    std::unordered_map<std::string, ClassInfo> class_info;

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

    const Type *pop_type() {
        const Type *result = types.back();
        types.pop_back();
        return result;
    }

public:

    // Public flags
    bool is_packed = false;
    // Don't mangle the name of the function. If this is true,
    //  function overloading won't be applicable for the function.
    bool nomangle = false;
    bool is_in_function = false;

    std::vector<bool> is_templated_stack;

    void push_str(std::string str) {
        strings.push_back(std::move(str));
    }

    void push_num(uint64_t num) { numbers.push_back(num); }

    template<typename T, typename...Args>
    void push_node(Args...args) { nodes.push_back(compiler->create_node<T>(args...)); }

    template<typename T, typename...Args>
    void push_type(Args...args) { types.push_back(compiler->create_type<T>(args...)); }

    void push_null_node() { nodes.push_back(nullptr); }
    void push_null_type() { types.push_back(nullptr); }

    bool in_class() { return !current_class.empty() || in_templated_class; }

    static int64_t get_i64(std::string num);
    static int32_t get_i32(std::string num);
    static int16_t get_i16(std::string num);
    static int8_t  get_i8 (std::string num);

    void push_is_templated(bool is_templated);
    bool pop_is_templated();

    // Rely on the general_counters list
    std::vector<ASTNode*> pop_args();
    std::vector<std::string> pop_strings();
    std::vector<const Type*> pop_types();

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
    void create_indexing();
    void create_logical_and();
    void create_logical_or();
    void create_identifier_type();
    void create_field_access();
    void create_inferred_definition();
    void create_size_of();
    void create_type_of();
    void create_type_name();
    void create_function_type();
    void create_malloc();
    void create_free();
    void prepare_constructor();
    void finish_constructor();
    void prepare_destructor();
    void create_pointer_field_access();
    void create_array_literal();
    void create_forced_cast();
    void create_func_ref();
    void prepare_copy_constructor();
    void finish_copy_constructor();
    void set_current_function();
    void create_reference_type();
    void create_type_alias();
    void create_identifier_lvalue();
    void create_templated_class_type();
    void create_is_type();
    void create_no_ref();
    void create_method_identifier();

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

    void create_operator(const std::string& position_name);
    void create_infix_operator();
    void create_postfix_operator();

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

    void push_counter();
    size_t pop_counter();
    void inc_counter();
    void push_var_arg(bool value);
    bool pop_var_arg();
    void create_function_call();
    void create_method_call();
    void create_expr_function_call();

    void register_class();
    void finish_class_declaration();
    void start_class_definition();
    void create_class();

    bool is_in_global_scope();

    TranslationUnitNode* construct_result();

    void finish_parsing();
    void create_missing_methods();
    void create_empty_method_if_doesnt_exist(const ClassType* cls, std::string&& name);

    void set_module_compiler(ModuleCompiler* compiler) { this->compiler = compiler; }
};

}
