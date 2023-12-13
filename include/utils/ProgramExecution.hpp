#pragma once

#include <string>
#include <vector>

namespace dua
{

struct ProgramExecution
{
    std::string std_out;
    int exit_code;
};

class TLEException : public std::exception {};

ProgramExecution execute_program(const std::string& program, const std::vector<std::string>& args = {}, long long time_limit = 2000);

}
