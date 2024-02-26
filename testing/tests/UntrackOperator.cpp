#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(untrack, untrack) {
    FileTestCasesRunner("untrack-operator.dua").run();
}

}
