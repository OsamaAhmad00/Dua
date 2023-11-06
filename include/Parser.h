#pragma once

#include <ModuleCompiler.h>
#include <AST/TranslationUnitNode.h>

class Parser
{
    ModuleCompiler& module_compiler;
public:
    Parser(ModuleCompiler& module_compiler) : module_compiler(module_compiler) {}
    TranslationUnitNode* parse(const std::string& str) const;
};