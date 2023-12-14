#include <utils/CodeGeneration.hpp>
#include <cstdlib>
#include <fstream>
#include <boost/algorithm/string/join.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <filesystem>
#include <utils/VectorOperators.hpp>
#include <utils/TextManipulation.hpp>
#include <ModuleCompiler.hpp>
#include <Preprocessor.hpp>
#include <boost/process.hpp>

namespace bp = boost::process;

namespace dua
{

std::string uuid()
{
    boost::uuids::random_generator generator;
    return boost::uuids::to_string(generator());
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

int run_clang(const std::vector<std::string>& args)
{
    // FIXME make it detect the available versions of clang

    std::string concatenated = " -Wno-override-module " + boost::algorithm::join(args, " ");

#ifdef _WIN32
    auto prefix = "clang";
#else
    // -lm = link with the math library
    auto prefix = "clang-17 -lm";
#endif

    return std::system((prefix + concatenated).c_str());
}

bool run_clang_on_llvm_ir(const strings& filename, const strings& code, const strings& args)
{
    auto directory = uuid();
    std::filesystem::create_directory(directory);
    auto old_path = std::filesystem::current_path();
    std::filesystem::current_path(directory);

    auto names = (filename + ".ll");

    try {
        generate_llvm_ir(names, code);
    } catch (std::exception& e) {
        // This assumes that the compiler has already reported the error.
        std::filesystem::current_path(old_path);
        std::filesystem::remove_all(directory);
        throw e;
    }

    std::filesystem::current_path(old_path);

    run_clang(strings { "-x", "ir" } + ("./" + directory + "/" + names) + args);

    std::filesystem::remove_all(directory);

    return true;
}

void compile(const strings& source_files, const strings& args)
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
        run_clang_on_llvm_ir(stripped, code, args);
    } catch (...) {
        exit(-1);
    }
}

}
