#include "DuaParser.h"
#include "DuaLexer.h"
#include "parsing/ParserFacade.hpp"

namespace dua
{

class ThrowExceptionErrorStrategy : public antlr4::DefaultErrorStrategy
{
    void reportError(antlr4::Parser *recognizer, const antlr4::RecognitionException &e) override {
        DefaultErrorStrategy::reportError(recognizer, e);
        report_error("Parsing error");
    }
};

TranslationUnitNode* ParserFacade::parse(const std::string& str) const
{
    antlr4::ANTLRInputStream input(str);

    // Create a lexer from the input
    DuaLexer lexer(&input);

    // Create a token stream from the lexer
    antlr4::CommonTokenStream tokens(&lexer);

    // Create a parser from the token stream
    DuaParser parser(&tokens);

    module_compiler.parser_assistant = &parser.assistant;

    auto handler = std::make_shared<ThrowExceptionErrorStrategy>();
    parser.setErrorHandler(handler);
    parser.set_module_compiler(&module_compiler);

    auto result = parser.parse();

    module_compiler.parser_assistant = nullptr;

    return result;
}

}
