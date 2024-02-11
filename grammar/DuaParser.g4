parser grammar DuaParser;

options {
    tokenVocab = DuaLexer;
}

@parser::postinclude {
#include <parsing/ParserAssistant.hpp>
}

@parser::members
{
public:

    ParserAssistant assistant;

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
    | type_alias
    ;

class_decl_or_def
    : class_declaration
    | class_definition
    ;

class_declaration
    : class_decl_no_semicolon ';' { assistant.finish_class_declaration(); }
    ;

class_decl_no_semicolon
    : Class identifier template_params_or_none { assistant.register_class(); }
    ;

class_definition
    : class_decl_no_semicolon class_optionals
       scope_begin { assistant.start_class_definition(); }
       class_elements_or_none scope_end { assistant.create_class(); }
    ;

// Only one of them can be present
class_optionals
    : optional_packed { assistant.push_type<IdentifierType>("Object"); }
    | optional_parent_class { assistant.is_packed = false; }
    ;

optional_parent_class
    : ':' identifier_type
    | /* empty */ { assistant.push_type<IdentifierType>("Object"); }
    ;

optional_packed
    : Packed        { assistant.is_packed = true;  }
    | /* empty */   { assistant.is_packed = false; }
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
    | copy_constructor
    | destructor
    | type_alias
    ;

constructor
    : { assistant.prepare_constructor(); } Constructor
        no_template
        '(' param_list ')' { assistant.push_var_arg(false); }
        { assistant.create_function_declaration(); } optional_fields_constructor_params
        { assistant.set_current_function(); } function_body { assistant.finish_constructor(); }
    | { assistant.prepare_constructor(); } Constructor
        no_template '(' param_list ')' ';' { assistant.push_var_arg(false); }
        { assistant.create_function_declaration(); }
    ;

copy_constructor
    : { assistant.prepare_copy_constructor(); } '=' Constructor
        no_template
        '(' param_list ')' { assistant.push_var_arg(false); }
        { assistant.create_function_declaration(); } optional_fields_constructor_params
        { assistant.set_current_function(); } function_body { assistant.finish_copy_constructor(); }
    | { assistant.prepare_copy_constructor(); } '=' Constructor
        no_template '(' param_list ')' ';' { assistant.push_var_arg(false); }
        { assistant.create_function_declaration(); }
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
    | Super { assistant.push_str("Super"); } '(' arg_list ')'
    ;

destructor
    : { assistant.prepare_destructor(); } Destructor { assistant.push_var_arg(false); }
        no_template
        { assistant.push_counter(); assistant.create_function_declaration(); } { assistant.set_current_function(); } function_body
    | { assistant.prepare_destructor(); } Destructor { assistant.push_var_arg(false); }
        no_template ';' { assistant.push_counter(); assistant.create_function_declaration(); }
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
    : variable_decl_optionals type identifier { assistant.create_variable_declaration(); }
    ;

variable_decl_optionals
    : static_or_none extern_or_none
    | extern_or_none static_or_none
    ;

static_or_none
    : Static      { assistant.is_static = true;  }
    | /* empty */ { assistant.is_static = false; }
    ;

extern_or_none
    : Extern      { assistant.is_extern = true;  }
    | /* empty */ { assistant.is_extern = false; }
    ;

// Every definition takes 4 arguments: type, name, expr, and args. Some of them may be empty (null or empty args)
variable_def_no_simicolon
    : static_or_none type '(' types_list var_arg_or_none ')' '*' identifier '=' identifier template_args_or_none { assistant.create_func_ref(); }
    | static_or_none type identifier '=' expression { assistant.push_counter(); assistant.create_variable_definition(); }
    | static_or_none Var  identifier '=' expression { assistant.push_counter(); assistant.create_inferred_definition(); }
    | static_or_none type identifier { assistant.push_null_node(); } optional_constructor_args { assistant.create_variable_definition(); }
    ;

optional_constructor_args
    : '(' arg_list ')'
    | /* empty */ { assistant.push_counter(); }
    ;

infix_op
    : '+'   { assistant.push_str("Addition"); }
    | '-'   { assistant.push_str("Subtraction"); }
    | '/'   { assistant.push_str("Division"); }
    | '*'   { assistant.push_str("Multiplication"); }
    | '%'   { assistant.push_str("Mod"); }
    | '='   { assistant.push_str("Assignment"); }
    | '&'   { assistant.push_str("BitwiseAnd"); }
    | '^'   { assistant.push_str("Xor"); }
    | '|'   { assistant.push_str("BitwiseOr"); }
    | '<<'  { assistant.push_str("LeftShift"); }
    | '>>'  { assistant.push_str("RightShift"); }
    | '>>>' { assistant.push_str("ArithmeticRightShift"); }
    | '<'   { assistant.push_str("LT"); }
    | '>'   { assistant.push_str("GT"); }
    | '<='  { assistant.push_str("LTE"); }
    | '>='  { assistant.push_str("GTE"); }
    | '=='  { assistant.push_str("EQ"); }
    | '!='  { assistant.push_str("NE"); }
    ;

postfix_op
    : '[]' { assistant.push_str("Indexing"); }
    ;

optional_declaration_specifier
    : Declaration
    |
    ;

function_decl_no_simicolon
    : optional_declaration_specifier function_decl_optionals type identifier template_params_or_none
        '(' param_list var_arg_or_none ')' { assistant.create_function_declaration(); }
    | static_or_none type Infix infix_op '(' param_list ')' { assistant.create_infix_operator(); }
    | static_or_none type Postfix postfix_op '(' param_list ')' { assistant.create_postfix_operator(); }
    ;

function_decl_optionals
    : static_or_none nomangle_or_none
    | nomangle_or_none static_or_none
    ;

no_template
    : /* empty */ { assistant.push_counter(); assistant.push_is_templated(false); }
    ;

template_params_or_none
    : '<' template_param_list '>' { assistant.push_is_templated(true); }
    | no_template
    ;

template_param_list @init { assistant.push_counter(); }
    : comma_separated_identifiers
    | /* empty */
    ;

comma_separated_identifiers
    : comma_separated_identifiers ',' identifier { assistant.inc_counter(); }
    | identifier { assistant.inc_counter(); }
    ;

nomangle_or_none
    : NoMangle      { assistant.nomangle = true;  }
    | /* empty */   { assistant.nomangle = false; }
    ;

function_declaration
    : function_decl_no_simicolon ';'
    ;

function_definition
    : function_decl_no_simicolon { assistant.set_current_function(); } function_body
    ;

function_body
    : block_statement { assistant.create_function_definition_block_body(); }
    | '=' expression ';' { assistant.create_function_definition_expression_body(); }
    ;

param_list @init { assistant.push_counter(); }
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
    : param { assistant.inc_counter(); }
    | comma_separated_params ',' param { assistant.inc_counter(); }
    ;

block_statement
    : scope_begin statements scope_end { assistant.create_block(); assistant.inc_statements(); }
    ;

statements
    : statements statement
    | /* empty */
    ;

scoped_statement
    : block_statement
    | { assistant.enter_scope(); } statement { assistant.create_block(); assistant.inc_statements(); }
    ;

// There is a corner case. a & b shouldn't be parsed as defining b to be of type a&,
//  but a & b = x should be parsed as a reference variable definition.
// The normal assignment defintion shouldn't have a higher precedence than the fucntion
//  reference definition, that's why we need to copy the function reference definition
//  here as well, and put it above both, so that it has a higher precedence.
statement_corner_case
    : static_or_none type '(' types_list var_arg_or_none ')' '*' identifier '=' identifier template_args_or_none ';' { assistant.create_func_ref(); }
    | static_or_none type identifier '=' expression ';' { assistant.push_counter(); assistant.create_variable_definition(); }
    | expression '&' expression ';' { assistant.create_binary_expr<BitwiseAndNode>(); assistant.create_expression_statement(); }
    ;

raw_or_none
    : Raw          { assistant.is_raw = false; }
    | /* empty */  { assistant.is_raw = true;  }
    ;

delete_brackets_or_none
    : '[]'        { assistant.is_array = true;  }
    | /* empty */ { assistant.is_array = false; }
    ;

statement
    : statement_corner_case
    | variable_decl_or_def
    | if_statement
    | for
    | while
    | do_while
    | block_statement
    | type_alias
    | return_statement
    | raw_or_none Delete delete_brackets_or_none expression ';' { assistant.create_free(); }
    | Continue { assistant.create_continue(); }
    | Break { assistant.create_break(); }
    | Move '(' identifier ')' ';' { assistant.call_destructor = true; assistant.create_move(); assistant.inc_statements(); }
    | expression_statement
    | ';'  { assistant.create_empty_statement(); }
    ;

expression
    : number
    | Char   { assistant.push_str($Char.text)  ; assistant.create_char_value();   }
    | String { assistant.push_str($String.text); assistant.create_string_value(); }
    | type '{' arg_list '}' { assistant.create_array_literal(); }
    | block_expression
    | if_expression
    | when_expression
    | '(' expression ')'
    | Move '(' identifier ')' { assistant.call_destructor = false; assistant.create_move(); }
    | '(' arg_list ')' type { assistant.create_temp_variable(); }
    | expression '.' function_name template_args_or_none '(' arg_list ')' { assistant.create_method_call(); }
    | expression '->' { assistant.create_dereference(); } function_name template_args_or_none '(' arg_list ')' { assistant.create_method_call(); }
    | full_function_identifier template_args_or_none '(' arg_list ')' { assistant.create_function_call(); }
    | expression '(' arg_list ')' { assistant.create_expr_function_call();  }
    | expression '[' expression ']' { assistant.create_indexing(); }
    | '(' '(' type ')' ')' expression { assistant.create_forced_cast(); }
    | '(' type ')' expression { assistant.create_cast(); }
    | raw_or_none New optional_new_size identifier_type optional_constructor_args { assistant.create_malloc(); }
    | raw_or_none New optional_new_size type { assistant.push_counter(); assistant.create_malloc(); }
    | SizeOf '(' expr_or_type ')'   { assistant.create_size_of();   }
    | TypeName '(' expr_or_type ')' { assistant.create_type_name(); }
    | DynamicName '(' expression ')' { assistant.push_null_type(); assistant.create_dynamic_name(); }
    | DynamicName '(' type ')' { assistant.push_null_node(); assistant.create_dynamic_name(); }
    | IsType '(' expr_or_type ',' expr_or_type ')' { assistant.create_is_type(); }
    | expression '.' identifier template_args_or_none { assistant.create_field_access(); }
    | expression { assistant.create_dereference(); } '->' identifier template_args_or_none { assistant.create_field_access(); }
    | '*' expression { assistant.create_dereference(); }
    | variable_identifier template_args_or_none { assistant.create_identifier_value(); }
    | expression '++' { assistant.create_post_inc(); }
    | expression '--' { assistant.create_post_dec(); }
    | '++' expression { assistant.create_pre_inc(); }
    | '--' expression { assistant.create_pre_dec(); }
    | '+'  expression  // do nothing
    | '-'  expression { assistant.create_unary_expr<NegativeExpressionNode>();          }
    | '!'  expression { assistant.create_unary_expr<NotExpressionNode>();               }
    | '~'  expression { assistant.create_unary_expr<BitwiseComplementExpressionNode>(); }
    | '&' expression  { assistant.create_address_of(); }
    | expression As type { assistant.create_dynamic_cast(); }
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
    | <assoc=right> expression '=' expression { assistant.create_assignment(); }
    | <assoc=right> expression '+='  expression  { assistant.create_compound_assignment<AdditionNode>(); }
    | <assoc=right> expression '-='  expression  { assistant.create_compound_assignment<SubtractionNode>(); }
    | <assoc=right> expression '*='  expression  { assistant.create_compound_assignment<MultiplicationNode>(); }
    | <assoc=right> expression '/='  expression  { assistant.create_compound_assignment<DivisionNode>(); }
    | <assoc=right> expression '%='  expression  { assistant.create_compound_assignment<ModNode>(); }
    | <assoc=right> expression '<<=' expression  { assistant.create_compound_assignment<LeftShiftNode>(); }
    | <assoc=right> expression '>>=' expression  { assistant.create_compound_assignment<RightShiftNode>(); }
    | <assoc=right> expression '>>>=' expression { assistant.create_compound_assignment<ArithmeticRightShiftNode>(); }
    | <assoc=right> expression '&='  expression  { assistant.create_compound_assignment<BitwiseAndNode>(); }
    | <assoc=right> expression '^='  expression  { assistant.create_compound_assignment<XorNode>(); }
    | <assoc=right> expression '|='  expression  { assistant.create_compound_assignment<BitwiseOrNode>(); }
    ;

optional_new_size
    : '[' expression ']' { assistant.is_array = true; }
    | /* empty */ { assistant.push_node<I64ValueNode>(1); assistant.is_array = false; }
    ;

expr_or_type
    : expression { assistant.create_type_of(); }
    | type
    ;

full_function_identifier
    : function_name
    | identifier_type '::' function_name { assistant.create_method_identifier(); }
    ;

variable_identifier
    : identifier
    | full_function_identifier
    ;

function_name
    : identifier
    | Constructor { assistant.push_str("constructor"); }
    | Destructor  { assistant.push_str("destructor") ; }
    ;

template_args_or_none
    : '<' types_list '>' { assistant.push_is_templated(true); }
    | '<' comma_terminated_types_list identifier { assistant.inc_counter(); } '<' types_list '>>'
       { assistant.push_is_templated(true); assistant.push_is_templated(true); assistant.create_identifier_type(); }
    | '<' comma_terminated_types_list identifier { assistant.inc_counter(); }
       '<' comma_terminated_types_list identifier { assistant.inc_counter(); } '<' types_list '>>>'
       { assistant.push_is_templated(true); assistant.push_is_templated(true); assistant.create_identifier_type(); }
       { assistant.push_is_templated(true); assistant.push_is_templated(true); assistant.create_identifier_type(); }
    | /* empty */ no_template
    ;

comma_terminated_types_list
    : types_list ','
    | /* empty */  { assistant.push_counter(); }
    ;

type_alias
    : TypeAlias identifier '=' type ';' { assistant.create_type_alias(); }
    ;

return_statement
    : Return expression ';' { assistant.create_return(); }
    | Return ';' { assistant.push_null_node(); assistant.create_return(); }
    ;

expression_statement
    : expression ';' { assistant.create_expression_statement(); }
    ;

if_statement  @init { assistant.enter_conditional(); assistant.inc_branches(); }
    : 'if' '(' expression ')' scoped_statement else_if_else_statement { assistant.create_if_statement(); }
    ;

else_if_else_statement
    : 'else' 'if' '(' expression ')' scoped_statement else_if_else_statement { assistant.inc_branches(); }
    | else_statement
    ;

else_statement
    : 'else' scoped_statement { assistant.set_has_else();  }
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
      ')' scoped_statement { assistant.create_for(); }
      ;

while
    : 'while' '(' expression_or_none_loop ')' scoped_statement { assistant.create_while(); }
    ;

do_while
    : 'do' scoped_statement 'while' '(' expression_or_none_loop ')' ';' { assistant.create_do_while(); }
    ;

arg_list @init { assistant.push_counter(); }
    : comma_separated_arguments
    | /* empty */
    ;

comma_separated_arguments
    : expression { assistant.inc_counter(); }
    | comma_separated_arguments ',' expression { assistant.inc_counter(); }
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

// A convinience production that pushes the identifier's text.
identifier
    : Identifier { assistant.push_str($Identifier.text); }
    ;

number
    : integer
    | float
    | True  { assistant.push_node<I8ValueNode>(1); }
    | False { assistant.push_node<I8ValueNode>(0); }
    | Null  { assistant.create_null_ptr(); }
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
    : comma_separated_types ',' type { assistant.inc_counter(); }
    | type { assistant.inc_counter(); }
    ;

types_list @init { assistant.push_counter(); }
    : comma_separated_types
    | /* empty */
    ;

type
    : '(' type ')'
    | type '(' types_list var_arg_or_none ')'   { assistant.create_function_type();  }
    | type '[' size ']'                         { assistant.create_array_type();     }
    | type '*'                                  { assistant.create_pointer_type();   }
    | type '&'                                  { assistant.create_reference_type(); }
    | TypeOf '(' expression ')'                 { assistant.create_type_of();        }
    | NoRef '(' type ')'                        { assistant.create_no_ref();         }
    | primitive_type
    | identifier_type
    ;

identifier_type
    : identifier template_args_or_none { assistant.create_identifier_type(); }
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
