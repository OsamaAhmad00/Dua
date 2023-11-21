#pragma once

#include <cstdlib>
#include <fstream>
#include <boost/algorithm/string/join.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <ModuleCompiler.h>

using strings = std::vector<std::string>;

strings operator+(const strings& v1, const strings& v2) {
    strings result(v1);
    result.insert(result.end(), v2.begin(), v2.end());
    return result;
}

strings operator+(const strings& v, const std::string& s) {
    strings result(v.size());
    for (size_t i = 0; i < v.size(); i++)
        result[i] = v[i] + s;
    return result;
}

strings operator+(const std::string& s, const strings& v) {
    strings result(v.size());
    for (size_t i = 0; i < v.size(); i++)
        result[i] = s + v[i];
    return result;
}

std::string uuid()
{
    boost::uuids::random_generator generator;
    return boost::uuids::to_string(generator());
}

std::string read_file(const std::string& name)
{
    std::ifstream stream(name);
    std::string result, temp;
    while (std::getline(stream, temp))
        result += temp + '\n';
    return result;
}

void generate_llvm_ir(const strings& filename, const strings& code)
{
    assert(filename.size() == code.size());
    for (size_t i = 0; i < filename.size(); i++) {
        // TODO This is a great candidate for multithreading
        dua::ModuleCompiler compiler(filename[i], code[i]);
        std::ofstream output(filename[i]);
        output << compiler.get_result();
        output.close();
    }
}

void run_clang(const std::vector<std::string>& args)
{
    std::string concatenated = boost::algorithm::join(args, " ");
    std::system(("clang -Wno-override-module " + concatenated).c_str());
}

void run_clang_on_llvm_ir(const strings& filename, const strings& code, const strings& args)
{
    auto directory = uuid();
    std::filesystem::create_directory(directory);
    auto old_path = std::filesystem::current_path();
    std::filesystem::current_path(directory);

    auto names = (filename + ".ll");
    generate_llvm_ir(names, code);

    std::filesystem::current_path(old_path);

    run_clang(strings { "-x", "ir" } + ("./" + directory + "/" + names) + args);

    std::filesystem::remove_all(directory);
}

void compile(const strings& source_files, const strings& args)
{
    size_t n = source_files.size();

    strings stripped(n);
    for (size_t i = 0; i < n; i++) {
        // without the .dua suffix
        stripped[i] = source_files[i].substr(0, source_files[i].size() - 4);
    }

    strings code(n);
    for (int i = 0; i < n; i++)
        code[i] = read_file(source_files[i]);

    return run_clang_on_llvm_ir(stripped, code, args);
}