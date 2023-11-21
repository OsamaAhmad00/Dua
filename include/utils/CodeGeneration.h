#pragma once

#include <utils/VectorOperators.h>

std::string uuid();
std::string read_file(const std::string& name);
void generate_llvm_ir(const strings& filename, const strings& code);
void run_clang(const std::vector<std::string>& args);
void run_clang_on_llvm_ir(const strings& filename, const strings& code, const strings& args);
void compile(const strings& source_files, const strings& args);
