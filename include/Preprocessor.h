#pragma once

#include <string>
#include <unordered_map>

namespace dua
{

class Preprocessor
{
    std::unordered_map<std::string, std::string> contents;
    std::string _process(const std::string& filename, const std::string& content);

public:

    std::string process(const std::string& filename, const std::string& content);
};

}