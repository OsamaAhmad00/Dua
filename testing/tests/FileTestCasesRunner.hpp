#include <string>
#include <vector>
#include <gtest/gtest.h>

namespace dua
{

class FileTestCasesRunner
{
    std::string filename;
    std::vector<std::string> args;

    const std::string TESTS_PATH = "../../../examples/";

public:

    explicit FileTestCasesRunner(std::string filename, std::vector<std::string> args = {})
        : filename(std::move(filename)), args(std::move(args)) {}

    void run();
};

}
