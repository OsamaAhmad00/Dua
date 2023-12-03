#include <utils/CodeGeneration.h>
#include <utils/termcolor.hpp>
#include <iostream>
#include <cstring>
#include <utils/TextManipulation.h>

using namespace dua;

bool is_dua_file(const std::string& str) {
    return ends_with(str, ".dua");
}

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        std::cout << termcolor::red << "Fatal error" <<
            termcolor::reset << ": no input files. Run " <<
            termcolor::green << argv[0] << " --help" <<
            termcolor::reset << " for details.\n";
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--help") == 0)
        {
            std::cout << "\nUSAGE: " <<
            termcolor::green << argv[0] <<
            termcolor::reset << " [options] file...\n\n";

            std::cout << "Popular Options:\n"
                         "  -S                      Generate assembly-code files (run compilation steps only)\n"
                         "  -c                      Generate object files\n"
                         "  -emit-llvm              Generate LLVM IR code files (used along with the -S flag)\n"
                         "  --target=<value>        Generate code for the given target (<value> = target triple)\n\n";

            std::cout << "Note: the Dua compiler is based on clang. This means that you can pass post-IR-generation "
                         "clang options\n";
            return 0;
        }
    }

    std::vector<std::string> args;
    std::vector<std::string> source_files;
    for (int i = 1; i < argc; i++) {
        if (is_dua_file(argv[i]))
            source_files.emplace_back(argv[i]);
        else
            args.emplace_back(argv[i]);
    }

    compile(source_files, args);

    return 0;
}