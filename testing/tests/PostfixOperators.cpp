#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(postfix, postfix) {
    FileTestCasesRunner("postfix-operators.dua").run();
}

}
