#pragma once

#include <utils/VectorOperators.hpp>

namespace dua
{

std::string uuid();
void generate_llvm_ir(const strings& filename, const strings& code, bool include_libdua = true);
int  run_clang(const std::vector<std::string>& args, bool include_libdua = true);
bool run_clang_on_llvm_ir(const strings& filename, const strings& code, const strings& args, bool include_libdua = true);
void compile(const strings& source_files, const strings& args, bool include_libdua = true);

}
