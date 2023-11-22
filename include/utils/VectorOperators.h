#pragma once

#include <string>
#include <vector>

namespace dua
{

using strings = std::vector<std::string>;

strings operator+(const strings& v1, const strings& v2);
strings operator+(const strings& v, const std::string& s);
strings operator+(const std::string& s, const strings& v);

}
