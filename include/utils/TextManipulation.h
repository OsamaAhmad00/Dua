#pragma once

#include <string>
#include <vector>

namespace dua
{

struct TestFile
{
    std::string common;
    std::vector<std::string> cases;
};

std::string read_file(const std::string& name);
std::string escape_characters(const std::string& str);

// Testing files related functions
TestFile split_cases(const std::string& str);
std::pair<std::string, std::string> split_header_body(const std::string& str);
std::string extract_header_element(const std::string& str, const std::string& name);

}