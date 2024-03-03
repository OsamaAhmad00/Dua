#include <utils/CodeGeneration.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string/join.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <utils/VectorOperators.hpp>
#include <utils/TextManipulation.hpp>
#include <ModuleCompiler.hpp>
#include <Preprocessor.hpp>
#include <boost/process.hpp>
#include <boost/filesystem.hpp>

#ifdef _WIN32
#include <stdlib.h>

#define WHERE "where "
#define NO_STDERR " 2>NUL"
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <unistd.h>
#include <limits.h>

#define WHERE "which "
#define NO_STDERR ""
#define POPEN popen
#define PCLOSE pclose
#endif

namespace bp = boost::process;
namespace fs = boost::filesystem;

namespace dua
{

std::string uuid()
{
    boost::uuids::random_generator generator;
    return boost::uuids::to_string(generator());
}

void generate_llvm_ir(const strings& filename, const strings& code, bool include_libdua)
{
    assert(filename.size() == code.size());
    for (size_t i = 0; i < filename.size(); i++) {
        // TODO This is a great candidate for multithreading
        dua::ModuleCompiler compiler(filename[i], code[i], include_libdua);
        std::ofstream output(filename[i]);
        output << compiler.get_result();
        output.close();
    }
}

std::string get_clang_name()
{
    std::string clang_versions[] = { "clang-17", "clang-16", "clang-15", "clang" };
    for (const auto& name : clang_versions) {
        std::string cmd = WHERE + name + NO_STDERR;
        std::array<char, 128> buffer{};
        std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(cmd.c_str(), "r"), PCLOSE);
        if (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            return name;
        }
    }
    std::string message = "Can't find clang on the system under any of these names:\n";
    for (auto& name : clang_versions)
        message += "  " + name;
    report_error(message);
    return "";
}

std::string get_program_location()
{
#ifdef _WIN32
    char* path;
    _get_pgmptr(&path);
    return path;
#else
    char path[PATH_MAX]; // create a buffer to store the path
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path)); // read the symbolic
    return path;
#endif
}

std::string get_libdua()
{
    auto program_path = std::filesystem::weakly_canonical(get_program_location());
    auto directory = program_path.parent_path();
    return "\"-L" + directory.string() + "\" -ldua";
}

int run_clang(const std::vector<std::string>& args, bool include_libdua)
{
     std::string concatenated = " -Wno-override-module --start-no-unused-arguments " + boost::algorithm::join(args, " ");
     if (include_libdua)
         concatenated += " " + get_libdua();

#ifdef _WIN32
    std::string system_specific_flags = "";
#else
    // -lm = link with the math library
    std::string system_specific_flags = "-lm ";
#endif

    return std::system((get_clang_name() + " " + system_specific_flags + concatenated).c_str());
}

bool run_clang_on_llvm_ir(const strings& filename, const strings& code, const strings& args, bool include_libdua, bool use_temp)
{
    auto directory = (use_temp ? std::filesystem::temp_directory_path().string() : "");

#ifndef _WIN32
    // If not Windows
    if (use_temp) directory += "/";
#endif

    directory += uuid();
    std::filesystem::create_directory(directory);
    auto old_path = std::filesystem::current_path();
    std::filesystem::current_path(directory);

    auto names = (filename + ".ll");

    try {
        // If the library is not included, no declarations should be included
        generate_llvm_ir(names, code, include_libdua);
    } catch (std::exception& e) {
        // This assumes that the compiler has already reported the error.
        std::filesystem::current_path(old_path);
        std::filesystem::remove_all(directory);
        throw e;
    }

    std::filesystem::current_path(old_path);

    run_clang((directory + "/" + names) + args, include_libdua);

    std::filesystem::remove_all(directory);

    return true;
}

void compile(const strings& source_files, const strings& args, bool include_libdua)
{
    size_t n = source_files.size();

    strings stripped(n);
    for (size_t i = 0; i < n; i++) {
        // without the path prefix .dua suffix
        std::string filename = std::filesystem::path(source_files[i]).filename().string();
        stripped[i] = filename.substr(0, filename.size() - 4);
    }

    Preprocessor preprocessor;
    strings code(n);

    try {
        for (int i = 0; i < n; i++)
            code[i] = preprocessor.process(source_files[i], read_file(source_files[i]));
        run_clang_on_llvm_ir(stripped, code, args, include_libdua);
    } catch (std::exception& e) {
        std::cerr << e.what() << '\n';
        exit(1);
    }
}

}
