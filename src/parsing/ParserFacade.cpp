#include "DuaParser.h"
#include "DuaLexer.h"
#include "parsing/ParserFacade.h"

namespace dua
{

TranslationUnitNode* ParserFacade::parse(const std::string& str) const
{
    antlr4::ANTLRInputStream input(str);

    // Create a lexer from the input
    DuaLexer lexer(&input);

    // Create a token stream from the lexer
    antlr4::CommonTokenStream tokens(&lexer);

    // Create a parser from the token stream
    DuaParser parser(&tokens);
    parser.set_module_compiler(&module_compiler);

    return parser.parse();
}

}
