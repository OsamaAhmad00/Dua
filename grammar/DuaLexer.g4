lexer grammar DuaLexer;

channels { CommentsChannel }

// Whitespaces
WS: [ \t\r\n]+ -> skip;

// Comments
SingleLineComment : '//' ~[\r\n]* '\r'? '\n' -> channel(CommentsChannel);
MultiLineComment  : '/*' .*? '*/'            -> channel(CommentsChannel);

Int: Digit+;
Digit: [0-9];

Identifier: LETTER (LETTER | '0'..'9')*;
fragment LETTER : [a-zA-Z_$];

Symbol: [\-+*/!=<>,]+;

String: '"' .*? '"';

OpenParen: '(';
CloseParen: ')';