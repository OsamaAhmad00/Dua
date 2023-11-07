lexer grammar DuaLexer;

channels { CommentsChannel }

// Whitespaces
WS: [ \t\r\n]+ -> skip;

// Comments
SingleLineComment : '//' ~[\r\n]* '\r'? '\n' -> channel(CommentsChannel);
MultiLineComment  : '/*' .*? '*/'            -> channel(CommentsChannel);

Integer: Digit+;
Digit: [0-9];

Identifier: LETTER (LETTER | '0'..'9')*;
fragment LETTER : [a-zA-Z_$];

Symbol: [\-+*/!=<>,]+;

String: '"' .*? '"';

Comma: ',';

Plus: '+';
Minus: '-';
Star: '*';
Slash: '/';
Mod: '%';
LeftShift: '<<';
RightShift: '>>';

Bang: '!';
Carret: '^';
Tilde: '~';

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
RightArrow: '->';
Simicolon: ';';

If: 'if';
Else: 'else';
When: 'when';
For: 'for';
Do: 'do';
Break: 'break';
Continue: 'continue';

VarArg: '...';

I64: ('i64' | 'long'  );
I32: ('i32' | 'int'   );
I16: ('i16' | 'short' );
I8 : ('i8'  | 'bool' | 'byte');

F64: ('f64' | 'double');
F32: ('f32' | 'float');

Void: 'void';
