#include <ModuleCompiler.h>
#include "parsing/Parser.h"
#include <AST/TranslationUnitNode.h>
#include <sstream>


ModuleCompiler::ModuleCompiler(const std::string &module_name, const std::string &code) :
    context(),
    module(module_name, context),
    builder(context),
    temp_builder(context),
    current_function(nullptr)
{
    Parser parser(*this);

    // Parse
    TranslationUnitNode* ast = parser.parse(code);

    // Generate LLVM IR
    ast->eval();

    // Print code to stdout
    module.print(llvm::outs(), nullptr);

    // Save code to file
    std::error_code error;
    llvm::raw_fd_ostream out(module_name + ".ll", error);
    module.print(out, nullptr);

    llvm::raw_string_ostream stream(result);
    module.print(stream, nullptr);
    result = stream.str();

    delete ast;
}
