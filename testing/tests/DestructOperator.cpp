#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(destruct, destruct) {
    FileTestCasesRunner("destruct-operator.dua").run();
}

}
