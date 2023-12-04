#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(sizeof_operator, sizeof_operator) {
    FileTestCasesRunner("sizeof-operator.dua").run();
}

}
