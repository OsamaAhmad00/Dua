#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(infix, infix) {
    FileTestCasesRunner("infix-operators.dua").run();
}

}
