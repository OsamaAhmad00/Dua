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
RightArrow: '->';
Simicolon: ';';

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

I64: ('i64' | 'long' );
I32: ('i32' | 'int'  );
I16: ('i16' | 'short');
I8 : ('i8'  | 'bool' | 'byte');

F64: ('f64' | 'double');
F32: ('f32' | 'float');

Void: 'void';

Null: 'null';
True: 'true';
False: 'false';

I64Val: Integer 'L';
I32Val: Integer;
I16Val: Integer 'S';
I8Val:  Integer 'B';
F64Val: Float 'L';
F32Val: Float;

Identifier: LETTER (LETTER | Digit)*;

fragment Integer: (Digit | NONZERO (Digit | '\'')*);
fragment Float: Integer '.' Digit*;
fragment Digit: [0-9];
fragment NONZERO: [1-9];
fragment LETTER: [a-zA-Z_$];

String: '"' .*? '"';

// Whitespaces
WS: [ \t\r\n]+ -> channel(99);

// Comments
SingleLineComment : '//' ~[\r\n]* '\r'? '\n' -> channel(CommentsChannel);
MultiLineComment  : '/*' .*? '*/'            -> channel(CommentsChannel);

