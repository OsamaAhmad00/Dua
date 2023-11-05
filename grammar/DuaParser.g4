parser grammar DuaParser;

options {
	tokenVocab = DuaLexer;
}

@parser::postinclude {
#include <Expression.h>
#include <string>
}

entry_point returns [Expression result]
    : e = expression EOF { $result = $e.result; }
    ;

expression returns [Expression result]
    : a = atom { $result = $a.result; }
    | l = list { $result = $l.result; }
    ;

// The list owning the expressions is responsible for deleting them.
atom returns [Expression result]
    : i = Identifier { $result = Expression(SExpressionType::IDENTIFIER, $i.text); }
    | s = Symbol     { $result = Expression(SExpressionType::SYMBOL, $s.text); }
    | s = String     {
        std::string str($s.text);
        $result = Expression(SExpressionType::STRING, str.substr(1, str.size() - 2));
    }
    | i = Int        { $result = Expression(std::stoll($i.text)); }
    ;

list returns [Expression result]
    : '(' l = list_entries ')' { $result = $l.result; }
    ;

list_entries returns [Expression result]
    :  /* empty */                    { $result = Expression(std::vector<Expression>()); }
    | l = list_entries e = expression { $l.result.list.push_back($e.result); $result = $l.result; }
    ;
