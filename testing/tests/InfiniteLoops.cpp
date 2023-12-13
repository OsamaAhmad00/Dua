#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(infinite_loops, infinite_loops) {
    FileTestCasesRunner("infinite-loops.dua").run();
}

}
