#include "Parser.h"
#include "DuaParser.h"
#include "DuaLexer.h"

Expression Parser::parse(const std::string& str) const
{
    antlr4::ANTLRInputStream input(str);

    // Create a lexer from the input
    Dua::DuaLexer lexer(&input);

    // Create a token stream from the lexer
    antlr4::CommonTokenStream tokens(&lexer);

    // Create a parser from the token stream
    Dua::DuaParser parser(&tokens);

    // Parse the entry-point production
    Expression result = parser.entry_point()->result;

    return result;
}
