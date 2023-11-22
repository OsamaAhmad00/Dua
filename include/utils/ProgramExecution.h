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

ProgramExecution execute_program(const std::string& program, const std::vector<std::string>& args = {});

}
