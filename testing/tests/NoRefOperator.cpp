#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(noref_operator, noref_operator) {
    FileTestCasesRunner("noref-operator.dua").run();
}

}
