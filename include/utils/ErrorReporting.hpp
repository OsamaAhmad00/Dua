#pragma once

#include <string>

namespace dua
{

class ModuleCompiler;

void report_error(const std::string& message);
void report_internal_error(const std::string& message);
void report_warning(const std::string& message);

void report_error(const std::string& message, ModuleCompiler* compiler);
void report_internal_error(const std::string& message, ModuleCompiler* compiler);
void report_warning(const std::string& message, ModuleCompiler* compiler);

}