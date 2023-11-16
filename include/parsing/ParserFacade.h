#pragma once

#include "ModuleCompiler.h"
#include "AST/TranslationUnitNode.h"

namespace dua
{

class ParserFacade
{
    ModuleCompiler& module_compiler;
public:
    ParserFacade(ModuleCompiler& module_compiler) : module_compiler(module_compiler) {}
    TranslationUnitNode* parse(const std::string& str) const;
};

}
