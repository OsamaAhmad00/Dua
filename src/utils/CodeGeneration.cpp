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
#include <boost/filesystem.hpp>

namespace bp = boost::process;
namespace fs = boost::filesystem;

namespace dua
{

std::string uuid()
{
    boost::uuids::random_generator generator;
    return boost::uuids::to_string(generator());
}

std::string get_clang_name()
{
#ifdef _WIN32
    return "clang";
#else
    // -lm = link with the math library
    return "clang-17 -lm";
#endif
}

void generate_llvm_ir(const strings& filename, const strings& code, bool append_declarations)
{
    assert(filename.size() == code.size());
    for (size_t i = 0; i < filename.size(); i++) {
        // TODO This is a great candidate for multithreading
        dua::ModuleCompiler compiler(filename[i], code[i], append_declarations);
        std::ofstream output(filename[i]);
        output << compiler.get_result();
        output.close();
    }
}

std::string get_dua_lib()
{
    std::string path = PROJECT_ROOT_DIR + "/bin";
    std::string file = path + "/dua.a";

    if (fs::exists(file))
        return file;

    if (!fs::exists(path)) {
        fs::create_directories(path);
    }

    strings obj_files;
    fs::directory_iterator end_iter;
    auto lib_dir = PROJECT_ROOT_DIR + "/lib";
    for(fs::directory_iterator dir_iter(lib_dir); dir_iter != end_iter; ++dir_iter)
    {
        if(fs::is_regular_file(dir_iter->status()))
        {
            auto name = dir_iter->path().filename().string();
            auto output_name = path + "/" + name;
            name = lib_dir + "/" + name;

            if (ends_with(name, ".c")) {
                output_name.back() = 'o';
                system((get_clang_name() + " " + name + " -c -o " + output_name).c_str());
                obj_files.push_back(output_name);
            } else if (ends_with(name, ".dua")) {
                output_name.pop_back();
                output_name.pop_back();
                output_name.pop_back();
                auto llvm_ir_name = output_name + "ll";
                output_name += 'o';
                Preprocessor p;
                auto code = read_file(name);
                code = p.process(name, code);
                generate_llvm_ir( { llvm_ir_name }, { code }, false);
                system((get_clang_name() + " -c -x ir -Wno-override-module " + llvm_ir_name + " -o " + output_name).c_str());
                fs::remove(llvm_ir_name);
                obj_files.push_back(output_name);
            }
            // else, ignore
        }
    }

    // TODO create and link against a dynamic library
    auto obj_files_concatenated = boost::algorithm::join(obj_files, " ");
    system(("ar r " + file + " " + obj_files_concatenated).c_str());

    for (auto& obj : obj_files)
        fs::remove(obj);

    return file;
}

int run_clang(const std::vector<std::string>& args)
{
    // FIXME make it detect the available versions of clang

     std::string concatenated = " -Wno-override-module " + boost::algorithm::join(args, " ") + " " + get_dua_lib();

    return std::system((get_clang_name() + concatenated).c_str());
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

    run_clang(("./" + directory + "/" + names) + args);

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
