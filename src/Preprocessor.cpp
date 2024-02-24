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
    auto p = absolute(std::filesystem::path(path));
    auto parent = p.parent_path();
    auto result = parent.string();
    return result;
}

std::string Preprocessor::process(const std::string &filename, const std::string &content)
{
    imported.clear();
    return _process(filename, content);
}

std::string Preprocessor::_process(const std::string &filename, const std::string &content)
{
    std::string keyword = "import";
    std::string result;

    // This is to avoid importing recursively forever
    auto it = imported.find(filename);
    if (it != imported.end()) {
        return "";
    }

    imported.insert(filename);

    auto old_path = std::filesystem::current_path();
    std::filesystem::current_path(get_parent_path(filename));

    size_t i = 0;
    while (true)
    {
        size_t keyword_start = i;
        while (keyword_start < content.size() &&
            (isspace(content[keyword_start]) || content[keyword_start] == '\n')) keyword_start++;

        if (keyword_start + keyword.size() >= content.size() || !starts_with(content, keyword, keyword_start))
            break;

        size_t keyword_end = keyword_start + keyword.size();
        while (keyword_end < content.size() && std::isspace(content[keyword_end])) keyword_end++;

        if (content.size() - keyword_end <= 2)
            report_error("Preprocessor: Can't have empty imports");
        if (content[keyword_end] != '"')
            report_error("Preprocessor: Imports must be specified between two \"\"");

        auto path_start = keyword_end + 1;
        size_t path_end = path_start + 1;
        while (path_end < content.size() && !(content[path_end] == '"' && content[path_end - 1] != '\\')) path_end++;

        if (path_end == content.size())
            report_error("Preprocessor: Imports must be specified between two \"\"");

        auto import_path = content.substr(path_start, path_end - path_start);
        import_path = get_full_path(import_path);

        result += content.substr(i, keyword_start - i);

        result += _process(import_path, read_file(import_path));

        i = path_end + 1;
    }

    result += content.substr(i, content.size() - i);

    std::filesystem::current_path(old_path);

    return result;
}

}