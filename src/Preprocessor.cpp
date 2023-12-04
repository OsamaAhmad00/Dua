#include <Preprocessor.hpp>
#include <utils/ErrorReporting.hpp>
#include <utils/TextManipulation.hpp>
#include <string>
#include <sstream>
#include <filesystem>


namespace dua
{

inline std::string get_full_path(const std::string& path) {
    return std::filesystem::absolute(std::filesystem::weakly_canonical(path)).string();
}

inline std::string get_parent_path(const std::string& path) {
    return std::filesystem::path(path).parent_path().string();
}

size_t find(const std::string& str, size_t start, const std::string& target, bool must_be_at_start=false)
{
    // TODO improve the matching algorithm
    for (size_t i = start; i < str.size(); i++)
    {
        bool matches = true;
        for (size_t j = 0; matches && j < target.size() && (i + j) < str.size(); j++)
            if (str[i + j] != target[j])
                matches = false;
        if (matches) {
            if (must_be_at_start) {
                size_t _i = i;
                while (_i > 0 && std::isspace(str[--_i]));
                if (!(_i == 0 || str[_i-1] == '\n'))
                    continue;
            }
            return i;
        }
    }
    return str.size();
}

std::string Preprocessor::process(const std::string &filename, const std::string &content)
{
    contents.clear();
    return _process(filename, content);
}

std::string Preprocessor::_process(const std::string &filename, const std::string &content)
{
    std::string keyword = "import";
    std::string result;

    // This is to avoid importing recursively forever
    auto it = contents.find(filename);
    if (it == contents.end())
        contents[filename] = "";
    else
        return it->second;

    auto old_path = std::filesystem::current_path();
    std::filesystem::current_path(get_parent_path(filename));

    size_t i = 0;
    while (true)
    {
        size_t keyword_start = find(content, i, keyword, true);

        if (keyword_start == content.size())
            break;

        size_t keyword_end = keyword_start + keyword.size();
        while (keyword_end < content.size() && std::isspace(content[keyword_end])) keyword_end++;

        if (content.size() - keyword_end <= 2)
            report_error("Preprocessor: Can't have empty imports");
        if (content[keyword_end] != '"')
            report_error("Preprocessor: Imports must be specified between two \"\"");

        auto path_start = keyword_end + 1;
        size_t path_end = find(content, path_start, "\"");  // including the "

        if (path_end == content.size())
            report_error("Preprocessor: Imports must be specified between two \"\"");

        auto import_path = content.substr(path_start, path_end - path_start);
        import_path = get_full_path(import_path);

        result += content.substr(i, keyword_start - i);

        result += process(import_path, read_file(import_path));

        i = path_end + 1;
    }

    result += content.substr(i, content.size() - i);

    std::filesystem::current_path(old_path);

    return contents[filename] = result;
}

}