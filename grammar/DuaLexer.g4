lexer grammar DuaLexer;

channels { CommentsChannel }

Comma: ',';

PlusEq: '+=';
PlusPlus: '++';
Plus: '+';
MinusEq: '-=';
MinusMinus: '--';
Minus: '-';
StarEq: '*=';
Star: '*';
SlashEq: '/=';
Slash: '/';
ModEq: '%=';
Mod: '%';
LeftShift: '<<';
LeftShiftEq: '<<=';
RightShift: '>>';
RightShiftEq: '>>=';
ArithmeticRightShift: '>>>';
ArithmeticRightShiftEq: '>>>=';
AndEq: '&=';
CarretEq: '^=';
OrEq: '|=';
Equals: '=';

Bang: '!';
Tilde: '~';
And: '&';
Carret: '^';
Or: '|';

AndAnd: '&&';
OrOr: '||';

LT : '<';
GT : '>';
EQ : '==';
NE : '!=';
LTE: '<=';
GTE: '>=';

OpenParen: '(';
CloseParen: ')';
OpenCurly: '{';
CloseCurly: '}';
OpenBracket: '[';
CloseBracket: ']';
IndexingOperator: '[]';
RightArrow: '->';
Simicolon: ';';

Extern: 'extern';
Static: 'static';

Class: 'class';
Dot: '.';
ScopeResolution: '::';
Super: 'Super';

If: 'if';
Else: 'else';
When: 'when';
For: 'for';
While: 'while';
Do: 'do';
Break: 'break';
Continue: 'continue';
Return: 'return';

Question: '?';
Colon: ':';

VarArg: '...';

Infix: 'infix';
Postfix: 'postfix';

I64: ('i64' | 'long' );
I32: ('i32' | 'int'  );
I16: ('i16' | 'short');
I8 : ('i8'  | 'bool' | 'byte');

F64: ('f64' | 'double');
F32: ('f32' | 'float');

Str: 'str';

Void: 'void';

Var: 'var';

NoMangle: 'nomangle';

SizeOf: 'sizeof';

TypeOf: 'typeof';

TypeName: 'typename';

DynamicName: 'dynamicname';

As: 'as';

ClassID: 'classid';

IsType: 'istype';

NoRef: 'noref';

TypeAlias: 'typealias';

Packed: 'packed';

Constructor: 'constructor';
Destructor: 'destructor';

New: 'new';
Delete: 'delete';

Null: 'null';
True: 'true';
False: 'false';

// This word is put here, just to prevent its
// existence in the source code. This is to
// make sure that the code is preprocessed first
Import: 'import';

I64Val: Integer;
I32Val: Integer 'I';
I16Val: Integer 'S';
I8Val:  Integer 'T';
F64Val: Float;
F32Val: Float 'F';

Integer: HexInteger | DecimalInteger | OctalInteger | BinaryInteger;

Identifier: LETTER (LETTER | DecimalDigit)*;

fragment HexInteger: '0x' (HexDigit | '\'')+;
fragment DecimalInteger: DecimalNonZeroDigit (DecimalDigit | '\'')*;
fragment OctalInteger: '0' (OctalDigit | '\'')*;  // Quick hack: 0 is counted as octal
fragment BinaryInteger: '0b' (BinaryDigit | '\'')+;

fragment Float: DecimalInteger '.' DecimalDigit*;

fragment HexDigit: [0-9a-fA-F];

fragment DecimalDigit: [0-9];
fragment DecimalNonZeroDigit: [1-9];

fragment OctalDigit: [0-7];

fragment BinaryDigit: [0-1];

fragment LETTER: [a-zA-Z_$];

String: '"' ('\\' . | '""' | ~["\\])* '"';

// Whitespaces
WS: [ \t\r\n]+ -> channel(99);

// Comments
SingleLineComment : '//' ~[\r\n]* '\r'? '\n'? -> channel(CommentsChannel);
MultiLineComment  : '/*' .*? '*/'             -> channel(CommentsChannel);

