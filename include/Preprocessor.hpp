#pragma once

#include <string>
#include <unordered_set>

namespace dua
{

class Preprocessor
{
    std::unordered_set<std::string> imported;

    std::string _process(const std::string& filename, const std::string& content);

public:

    std::string process(const std::string& filename, const std::string& content);
};

}