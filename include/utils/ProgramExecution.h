#pragma once

#include <string>
#include <vector>

int get_exit_code(const std::string& program, const std::vector<std::string>& args = {});
std::string get_stdout(const std::string& program, const std::vector<std::string>& args = {});
