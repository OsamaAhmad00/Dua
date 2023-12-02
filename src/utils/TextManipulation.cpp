#include <utils/TextManipulation.h>
#include <fstream>
#include <iostream>
#include <utils/ErrorReporting.h>
#include <boost/regex.hpp>

namespace dua
{

std::string read_file(const std::string& name)
{
    std::string result;
    std::ifstream stream(name);

    if (stream)
    {
        std::string temp;
        while (std::getline(stream, temp))
            result += temp + '\n';
    } else {
        report_internal_error("Couldn't read the contents of the file at " + name);
    }
    return result;
}

std::string escape_characters(const std::string& str)
{
    if (str.empty()) return str;

    std::string result;
    int i = 0;
    for (; i < str.size() - 1; i++) {
        if (str[i] == '\\') {
            i++;
            switch (str[i]) {
                case '\'': result.push_back('\''); break;
                case '"': result.push_back('\"');  break;
                case '?': result.push_back('\?');  break;
                case '\\': result.push_back('\\'); break;
                case 'a': result.push_back('\a');  break;
                case 'b': result.push_back('\b');  break;
                case 'f': result.push_back('\f');  break;
                case 'n': result.push_back('\n');  break;
                case 'r': result.push_back('\r');  break;
                case 't': result.push_back('\t');  break;
                case 'v': result.push_back('\v');  break;
                default: report_error(std::string("Undefined escape character: ") + str[i]);
            }
        } else result.push_back(str[i]);
    }
    if (i == str.size() - 1) {
        if (str.back() == '\\')
            report_error("Last character can't be a non-escaped \\.");
        else
            result.push_back(str.back());
    }
    return result;
}

TestFile split_cases(const std::string& str)
{
    TestFile result;
    std::vector<size_t> indices;
    boost::regex block_regex("\\s*//\\s*(?i:Case).*?");
    boost::sregex_iterator block_iter(str.begin(), str.end(), block_regex);
    boost::sregex_iterator block_end;
    while (block_iter != block_end) {
        indices.push_back(block_iter->position());
        block_iter++;
    }
    indices.push_back(str.size());

    result.common = str.substr(0, indices.front());
    for (size_t i = 0; i < indices.size() - 1; i++) {
        size_t len = indices[i + 1] - indices[i];
        result.cases.push_back(str.substr(indices[i], len));
    }

    return result;
}

std::pair<std::string, std::string> split_header_body(const std::string& str)
{
    size_t split = -1;
    for (size_t i = 0; i < str.size(); i++) {
        split = i;
        while (i < str.size() && std::isspace(str[i])) i++;
        if (!(i < str.size() - 1 && str[i] == '/' && str[i+1] == '/')) break;
        while (i < str.size() && str[i] != '\n') i++;
    }

    return {
        str.substr(0, split),
        str.substr(split)
    };
}

std::string extract_header_element(const std::string& str, const std::string& name)
{
    boost::regex name_regex("//\\s*(?i:" + name + ")\\s*([^\\n]*)");
    if (boost::smatch match; boost::regex_search(str, match, name_regex))
        return match[1];
    return "";
}

bool header_has_flag(const std::string& str, const std::string& flag)
{
    boost::regex name_regex("//\\s*(?i:" + flag + ")\\s*([^\\n]*)");
    boost::smatch match;
    return boost::regex_search(str, match, name_regex);
}

bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

}
