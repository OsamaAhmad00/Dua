#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(move_operator, move_operator) {
    FileTestCasesRunner("move-operator.dua").run();
}

}
