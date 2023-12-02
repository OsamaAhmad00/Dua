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
    : module EOF { assistant.finish_parsing(); }
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
    | class_decl_or_def
    ;

class_decl_or_def
    : class_declaration
    | class_definition
    ;

class_declaration
    : class_decl_no_semicolon ';' { assistant.finish_class_declaration(); }
    ;

class_decl_no_semicolon
    : Class identifier { assistant.register_class(); }
    ;

class_definition
    : class_decl_no_semicolon
       scope_begin { assistant.start_class_definition(); }
       class_elements_or_none scope_end { assistant.create_class(); }
    ;

class_elements_or_none
    : class_elements
    | /* empty */
    ;

class_elements
    : class_elements class_element
    | class_element
    ;

class_element
    : variable_decl_or_def
    | function_decl_or_def
    | constructor
    | destructor
    ;

constructor
    : { assistant.prepare_constructor(); } Constructor '(' param_list ')' { assistant.push_var_arg(false); }
        { assistant.create_function_declaration(); } optional_fields_constructor_params function_body
    ;

optional_fields_constructor_params
    : ':' fields_constructor_params
    | /* empty */
    ;

fields_constructor_params
    : fields_constructor_params ',' field_constructor_params { assistant.add_field_constructor_args(); }
    | field_constructor_params { assistant.add_field_constructor_args(); }
    ;

field_constructor_params
    : identifier '(' arg_list ')'
    ;

destructor
    : { assistant.prepare_destructor(); }  Destructor '(' ')' { assistant.push_var_arg(false); }
        { assistant.enter_arg_list(); assistant.create_function_declaration(); } function_body
    ;

variable_decl_or_def
    : variable_decl_or_def_no_simicolon ';'
    ;

// Variable definition is put first so that it has higher precedence.
//  This is to have no-args class definitinos to be considered as
//  definitions instead of declarations.
variable_decl_or_def_no_simicolon
    : variable_def_no_simicolon
    | variable_decl_no_simicolon
    ;

function_decl_or_def
    : function_declaration
    | function_definition
    ;

variable_decl_no_simicolon
    : type identifier { assistant.create_variable_declaration(); }
    ;

// Every definition takes 4 arguments: type, name, expr, args. Some of them may be empty (null or empty args)
variable_def_no_simicolon
    : type identifier '=' expression { assistant.enter_arg_list(); assistant.create_variable_definition(); }
    | Var  identifier '=' expression { assistant.enter_arg_list(); assistant.create_inferred_definition(); }
    | type identifier { assistant.push_null_node(); } optional_constructor_args { assistant.create_variable_definition(); }
    ;

optional_constructor_args
    : '(' arg_list ')'
    | /* empty */ { assistant.enter_arg_list(); }
    ;

function_decl_no_simicolon
    : type identifier
        '(' param_list var_arg_or_none ')' { assistant.create_function_declaration(); }
    ;

function_declaration
    : function_decl_no_simicolon ';'
    ;

function_definition
    : function_decl_no_simicolon function_body
    ;

function_body
    : block_statement { assistant.create_function_definition_block_body(); }
    | '=' expression ';' { assistant.create_function_definition_expression_body(); }
    ;

param_list @init { assistant.enter_arg_list(); }
    : comma_separated_params
    | /* empty */
    ;

var_arg_or_none
    : ',' '...'   { assistant.push_var_arg(true);  }
    | /* empty */ { assistant.push_var_arg(false); }
    ;

param
    : type identifier
    ;

comma_separated_params
    : param { assistant.inc_args(); }
    | comma_separated_params ',' param { assistant.inc_args(); }
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
    | Delete expression ';' { assistant.create_free(); }
    | Continue { assistant.create_continue(); }
    | Break { assistant.create_break(); }
    | ';'  { assistant.create_empty_statement(); }
    ;

expression
    : number
    | String { assistant.push_str($String.text); assistant.create_string_value(); }
    | expression '(' arg_list ')' { assistant.create_function_call(); }
    | block_expression
    | if_expression
    | when_expression
    | cast_expression
    | '(' expression ')'
    | New type optional_constructor_args       { assistant.create_malloc(); }
    | SizeOf '(' type ')'         { assistant.create_size_of_type();        }
    | SizeOf '(' expression ')'   { assistant.create_size_of_expression();  }
    | TypeName '(' type ')'       { assistant.create_typename_type();       }
    | TypeName '(' expression ')' { assistant.create_typename_expression(); }
    | lvalue '++' { assistant.create_post_inc(); }
    | lvalue '--' { assistant.create_post_dec(); }
    | '+'  expression  // do nothing
    | '++' lvalue { assistant.create_pre_inc(); }
    | '--' lvalue { assistant.create_pre_dec(); }
    | '-'  expression { assistant.create_unary_expr<NegativeExpressionNode>();          }
    | '!'  expression { assistant.create_unary_expr<NotExpressionNode>();               }
    | '~'  expression { assistant.create_unary_expr<BitwiseComplementExpressionNode>(); }
    | '&' lvalue  // the lvalue production has loaded the address already, there is nothing to be done here.
    | expression '*' expression  { assistant.create_binary_expr<MultiplicationNode>(); }
    | expression '/' expression  { assistant.create_binary_expr<DivisionNode>();       }
    | expression '%' expression  { assistant.create_binary_expr<ModNode>();            }
    | expression '+' expression  { assistant.create_binary_expr<AdditionNode>();       }
    | expression '-' expression  { assistant.create_binary_expr<SubtractionNode>();    }
    | expression '<<' expression { assistant.create_binary_expr<LeftShiftNode>(); }
    | expression '>>' expression { assistant.create_binary_expr<RightShiftNode>(); }
    | expression '>>>' expression { assistant.create_binary_expr<ArithmeticRightShiftNode>(); }
    | expression '<'  expression { assistant.create_binary_expr<LTNode>();  }
    | expression '>'  expression { assistant.create_binary_expr<GTNode>();  }
    | expression '<=' expression { assistant.create_binary_expr<LTENode>(); }
    | expression '>=' expression { assistant.create_binary_expr<GTENode>(); }
    | expression '==' expression { assistant.create_binary_expr<EQNode>();  }
    | expression '!=' expression { assistant.create_binary_expr<NENode>();  }
    | expression '&'  expression { assistant.create_binary_expr<BitwiseAndNode>(); }
    | expression '^'  expression { assistant.create_binary_expr<XorNode>(); }
    | expression '|'  expression { assistant.create_binary_expr<BitwiseOrNode>(); }
    | expression '&&' expression { assistant.create_logical_and(); }
    | expression '||' expression { assistant.create_logical_or(); }
    | expression '?' expression ':' expression { assistant.create_ternary_operator(); }
    | lvalue '='   expression  { assistant.create_assignment(); }
    | lvalue '+='  expression  { assistant.create_compound_assignment<AdditionNode>(); }
    | lvalue '-='  expression  { assistant.create_compound_assignment<SubtractionNode>(); }
    | lvalue '*='  expression  { assistant.create_compound_assignment<MultiplicationNode>(); }
    | lvalue '/='  expression  { assistant.create_compound_assignment<DivisionNode>(); }
    | lvalue '%='  expression  { assistant.create_compound_assignment<ModNode>(); }
    | lvalue '<<=' expression  { assistant.create_compound_assignment<LeftShiftNode>(); }
    | lvalue '>>=' expression  { assistant.create_compound_assignment<RightShiftNode>(); }
    | lvalue '>>>=' expression { assistant.create_compound_assignment<ArithmeticRightShiftNode>(); }
    | lvalue '&='  expression  { assistant.create_compound_assignment<BitwiseAndNode>(); }
    | lvalue '^='  expression  { assistant.create_compound_assignment<XorNode>(); }
    | lvalue '|='  expression  { assistant.create_compound_assignment<BitwiseOrNode>(); }
    | lvalue { assistant.create_loaded_lvalue(); }
    ;

cast_expression
    : '(' type ')' expression { assistant.create_cast(); }
    ;

return_statement
    : Return expression ';' { assistant.create_return(); }
    ;

expression_statement
    : expression ';' { assistant.create_expression_statement(); }
    ;

if_statement  @init { assistant.enter_conditional(); assistant.inc_branches(); }
    : 'if' '(' expression ')' statement else_if_else_statement { assistant.create_if_statement(); }
    ;

else_if_else_statement
    : 'else' 'if' '(' expression ')' statement else_if_else_statement { assistant.inc_branches(); }
    | else_statement
    ;

else_statement
    : 'else' statement { assistant.set_has_else();  }
    | /* empty */
    ;

if_expression @init {
    assistant.enter_conditional();
    assistant.inc_branches();
    assistant.set_has_else();
}
    : 'if' '(' expression ')' expression else_if_expression 'else' expression
        { assistant.push_str("if"); assistant.create_if_expression(); }
    ;

else_if_expression
    : 'else' 'if' '(' expression ')' expression else_if_expression { assistant.inc_branches(); }
    | /* empty */
    ;

when_expression @init {
    assistant.enter_conditional();
    assistant.set_has_else();
}
    : 'when' scope_begin when_list scope_end { assistant.leave_scope(); }
    ;

when_list
    : when_list_no_else ',' 'else' '->' expression { assistant.push_str("when"); assistant.create_if_expression(); }
    ;

when_list_no_else
    : when_item
    | when_list_no_else ',' when_item
    ;

when_item
    : expression '->' expression { assistant.inc_branches(); }
    ;

// Used for for loops
comma_separated_multi_variable_decl_or_def
    : comma_separated_multi_variable_decl_or_def
      ',' variable_decl_or_def_no_simicolon
    | variable_decl_or_def_no_simicolon
    ;


comma_separated_multi_variable_decl_or_def_or_none
    : comma_separated_multi_variable_decl_or_def
    | /* empty */
    ;

for @init { assistant.enter_scope(); }
    : 'for'
      '(' comma_separated_multi_variable_decl_or_def_or_none
      ';' expression_or_none_loop
      ';' expression_or_none_loop
      ')' statement { assistant.create_for(); }
      ;

while
    : 'while' '(' expression_or_none_loop ')' statement { assistant.create_while(); }
    ;

do_while
    : 'do' statement 'while' '(' expression_or_none_loop ')' ';' { assistant.create_do_while(); }
    ;

arg_list @init { assistant.enter_arg_list(); }
    : comma_separated_arguments
    | /* empty */
    ;

comma_separated_arguments
    : expression { assistant.inc_args(); }
    | comma_separated_arguments ',' expression { assistant.inc_args(); }
    ;

// If none, push 1. If used as a condition, equals true. If used as
// a statement, equals an expression statement with no side effect.
expression_or_none_loop
    : expression
    | /* empty */ { assistant.push_node<I8ValueNode>(1); }
    ;

block_expression
    : scope_begin statements expression scope_end { assistant.inc_statements(); assistant.create_block(); }
    ;

lvalue
    : '(' lvalue ')'
    | Identifier { assistant.push_node<VariableNode>($Identifier.text); }
    | lvalue '[' expression ']' { assistant.create_array_indexing(); }
    | '*' expression { assistant.create_dereference(); }
    | lvalue '.' identifier { assistant.create_field_access(); }
    | lvalue { assistant.create_loaded_lvalue(); } '->' identifier { assistant.create_field_access(); }
    ;

// A convinience production that pushes the identifier's text.
identifier
    : Identifier { assistant.push_str($Identifier.text); }
    ;

number
    : integer
    | float
    | True  { assistant.push_node<I8ValueNode>(1); }
    | False { assistant.push_node<I8ValueNode>(0); }
    | Null  { assistant.push_node<I8ValueNode>(0); }  // TODO you might make it a little more sophesticated.
    ;

size
    : I64Val { assistant.push_num(assistant.get_i64($I64Val.text)); }
    | I32Val { assistant.push_num(assistant.get_i64($I32Val.text)); }
    | I16Val { assistant.push_num(assistant.get_i64($I16Val.text)); }
    | I8Val  { assistant.push_num(assistant.get_i64($I8Val.text )); }
    ;

integer
    : I64Val { assistant.push_node<I64ValueNode>(assistant.get_i64($I64Val.text)); }
    | I32Val { assistant.push_node<I32ValueNode>(assistant.get_i32($I32Val.text)); }
    | I16Val { assistant.push_node<I16ValueNode>(assistant.get_i16($I16Val.text)); }
    | I8Val  { assistant.push_node<I8ValueNode >(assistant.get_i8 ($I8Val.text )); }
    ;

float
    : F64Val { assistant.push_node<F64ValueNode>(stod($F64Val.text)); }
    | F32Val { assistant.push_node<F32ValueNode>(stof($F32Val.text)); }
    ;

comma_separated_types
    : comma_separated_types ',' type { assistant.inc_args(); }
    | type { assistant.inc_args(); }
    ;

types_list @init { assistant.enter_arg_list(); }
    : comma_separated_types
    | /* empty */
    ;

type
    : primitive_type
    | class_type
    | type '(' types_list var_arg_or_none ')'   { assistant.create_function_type(); }
    | type '[' size ']'                         { assistant.create_array_type();    }
    | type '*'                                  { assistant.create_pointer_type();  }
    | TypeOf '(' expression ')'                 { assistant.create_type_of();       }
    ;

class_type
    : identifier { assistant.create_class_type(); }
    ;

primitive_type
    : I64  { assistant.push_type<I64Type>();  }
    | I32  { assistant.push_type<I32Type>();  }
    | I16  { assistant.push_type<I16Type>();  }
    | I8   { assistant.push_type<I8Type> ();  }
    | F64  { assistant.push_type<F64Type>();  }
    | F32  { assistant.push_type<F32Type>();  }
    | Str  { assistant.push_type<I8Type>(); assistant.create_pointer_type(); }
    | Void { assistant.push_type<VoidType>(); }
    ;

scope_begin: '{' { assistant.enter_scope(); };

// We don't call leave scope here, instead, it's up to the
//  assistant to determine when the scope ends (usually
//  ends when a block construction finishes).
scope_end:   '}';
