#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(construct, construct) {
    FileTestCasesRunner("construct-operator.dua").run();
}

}
