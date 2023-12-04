#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(Variables, Variables) {
    FileTestCasesRunner("variables.dua").run();
}

}
