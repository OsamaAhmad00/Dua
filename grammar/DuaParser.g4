parser grammar DuaParser;

options {
    tokenVocab = DuaLexer;
}

@parser::postinclude {
#include <parsing/ParserAssistant.h>
}

@parser::members
{
private:

    ParserAssistant assistant;

public:

    void set_module_compiler(ModuleCompiler* compiler) {
        assistant.set_module_compiler(compiler);
    }

    TranslationUnitNode* parse() {
        starting_symbol();
        return assistant.construct_result();
    }
}

starting_symbol
    : module EOF
    ;

module
    : global_elements
    | /* empty */
    ;

global_elements
    : global_elements global_element
    | /* empty */
    ;

global_element
    : variable_decl_or_def
    | function_decl_or_def
    ;

variable_decl_or_def
    : variable_decl_or_def_no_simicolon ';'
    ;

variable_decl_or_def_no_simicolon
    : variable_decl_no_simicolon
    | variable_def_no_simicolon
    ;

function_decl_or_def
    : function_declaration
    | function_definition
    ;

variable_decl_no_simicolon
    : type identifier { assistant.create_variable_declaration(); }
    ;

variable_def_no_simicolon
    : type identifier '=' expression { assistant.create_variable_definition(); }
    ;

function_decl_no_simicolon
    : type identifier
        '(' param_list ')' { assistant.create_function_declaration(); }
    ;

function_declaration
    : function_decl_no_simicolon ';'
    ;

function_definition
    : function_decl_no_simicolon block_statement { assistant.create_function_definition_block_body(); }
    | function_decl_no_simicolon '=' expression ';' { assistant.create_function_definition_expression_body(); }
    ;

param_list
    : comma_separated_params var_arg_or_none
    | /* empty */ { assistant.param_count = 0; assistant.is_var_arg = false; }
    ;

var_arg_or_none
    : ',' '...'   { assistant.is_var_arg = true;  }
    | /* empty */ { assistant.is_var_arg = false; }
    ;

param
    : type identifier
    ;

comma_separated_params
    : param { assistant.param_count = 1; }
    | comma_separated_params ',' param { assistant.param_count += 1; }
    ;

block_statement
    : scope_begin statements scope_end { assistant.create_block(); assistant.inc_statements(); }
    ;

statements
    : statements statement
    | /* empty */
    ;

statement
    : if_statement
    | for
    | while
    | do_while
    | block_statement
    | expression_statement
    | variable_decl_or_def
    | return_statement
    | Break
    | Continue
    | ';'  // empty statement
    ;

expression
    : number
    | String
    | variable
    | block_expression
    | function_call
    | if_expression
    | when_expression
    | '(' expression ')'
    | expression '++'
    | expression '--'
    | '++' expression
    | '--' expression
    | '-'  expression
    | '+'  expression  // do nothing
    | '!'  expression
    | '~'  expression
    | '&' lvalue
    | expression '*' expression
    | expression '/' expression
    | expression '%' expression
    | expression '+' expression
    | expression '-' expression
    | expression '<<' expression
    | expression '>>' expression
    | expression '<'  expression
    | expression '>'  expression
    | expression '<=' expression
    | expression '>=' expression
    | expression '==' expression
    | expression '!=' expression
    | expression '&'  expression
    | expression '^'  expression
    | expression '|'  expression
    | expression '&&' expression
    | expression '||' expression
    | expression '?' expression ':' expression  // ternary conditional
    | lvalue '='   expression { assistant.create_assignment(); }
    | lvalue '+='  expression
    | lvalue '-='  expression
    | lvalue '*='  expression
    | lvalue '/='  expression
    | lvalue '%='  expression
    | lvalue '<<=' expression
    | lvalue '>>=' expression
    | lvalue '&='  expression
    | lvalue '^='  expression
    | lvalue '|='  expression
    ;

return_statement
    : Return expression ';' { assistant.create_return(); }
    ;

expression_statement
    : expression ';' { assistant.create_expression_statement(); }
    ;

if_statement  @init { assistant.enter_conditional(); assistant.inc_branches(); }
    : 'if' '(' expression ')' statement else_if_else_statement { assistant.create_if(); }
    ;

else_if_else_statement
    : 'else' 'if' '(' expression ')' statement else_if_else_statement { assistant.inc_branches(); }
    | else_statement
    ;

else_statement
    : 'else' statement { assistant.has_else = true;  }
    | /* empty */      { assistant.has_else = false; }
    ;

// Expressions don't increase the statements counter.
// The IfNode decreases the statements count for each
//  branch it takes, assuming that it's a statement.
//  To cancel this effect for expressions, we increment
//  the statements counter once for each branch.
if_expression @init {
    assistant.enter_conditional();
    assistant.inc_branches();
    assistant.inc_statements();
    assistant.has_else = true;
}
    : 'if' '(' expression ')' expression else_if_expression 'else' expression  { assistant.create_if(); }
    ;

else_if_expression
    : 'else' 'if' '(' expression ')' expression else_if_expression { assistant.inc_branches(); assistant.inc_statements(); }
    | /* empty */
    ;

when_expression @init {
    assistant.enter_conditional();
    assistant.has_else = true;
}
    : 'when' scope_begin when_list scope_end { assistant.leave_scope(); }
    ;

when_list
    : when_list_no_else ',' 'else' '->' expression { assistant.create_if(); }
    ;

when_list_no_else
    : when_item
    | when_list_no_else ',' when_item
    ;

when_item
    : expression '->' expression { assistant.inc_branches(); assistant.inc_statements(); }
    ;

comma_separated_multi_variable_decl_or_def
    : comma_separated_multi_variable_decl_or_def
      ',' variable_decl_or_def_no_simicolon
    | variable_decl_or_def_no_simicolon
    ;


comma_separated_multi_variable_decl_or_def_or_none
    : comma_separated_multi_variable_decl_or_def
    | /* empty */
    ;

for
    : 'for'
      '(' comma_separated_multi_variable_decl_or_def_or_none
      ';' expression_or_none_loop
      ';' expression_or_none
      ')' statement
    ;

while
    : 'while' '(' expression_or_none_loop ')' statement { assistant.create_while(); }
    ;

do_while
    : 'do' statement 'while' '(' expression_or_none_loop ')' ';'
    ;

arg_list @init { assistant.enter_fun_call(); }
    : comma_separated_arguments
    | /* empty */
    ;

comma_separated_arguments
    : expression { assistant.inc_args(); }
    | comma_separated_arguments ',' expression { assistant.inc_args(); }
    ;

// If none, push true
expression_or_none_loop
    : expression
    | /* empty */ { assistant.push_node<I8ValueNode>(1); }
    ;

expression_or_none
    : expression
    | /* empty */
    ;

block_expression
    : scope_begin statements expression scope_end { assistant.inc_statements(); assistant.create_block(); }
    ;

function_call
    : identifier '(' arg_list ')' { assistant.create_function_call(); }
    ;

variable
    : Identifier { assistant.push_node<VariableNode>($Identifier.text); }
//    | '*' expression  // Not supported for now
    ;

lvalue
    : Identifier { assistant.push_node<VariableNode>($Identifier.text, true); }
//    | '*' expression  // Not supported for now
    ;

// A convinience production that pushes the identifier's text.
identifier
    : Identifier { assistant.push_str($Identifier.text); }
    ;

number
    : integer
    | float
    | True
    | False
    | Null
    ;

integer
    : I64Val { assistant.push_node<I64ValueNode>(stol($I64Val.text)); }
    | I32Val { assistant.push_node<I32ValueNode>(stoi($I32Val.text)); }
    | I16Val { assistant.push_node<I16ValueNode>(stoi($I16Val.text)); }
    | I8Val  { assistant.push_node<I8ValueNode >(stoi($I8Val.text )); }
    ;

float
    : F64Val
    | F32Val
    ;

type
    : primitive_type
    | Identifier            // User-defined types
    | type '[' integer ']'  // array types
    | type '*'              // pointer types
    ;

primitive_type
    : I64  { assistant.push_type<I64Type>(); }
    | I32  { assistant.push_type<I32Type>(); }
    | I16  { assistant.push_type<I16Type>(); }
    | I8   { assistant.push_type<I8Type> (); }
    | F64  { assistant.push_type<F64Type>(); }
    | F32  { assistant.push_type<F32Type>(); }
    | Void
    ;

scope_begin: '{' { assistant.enter_scope(); };

// We don't call leave scope here, instead, it's up to the
//  assistant to determine when the scope will end. (usually
//  ends when constructing a block.
scope_end:   '}';
