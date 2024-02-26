#include "FileTestCasesRunner.hpp"

namespace dua
{

TEST(teleport, teleport) {
    FileTestCasesRunner("teleport-operator.dua").run();
}

}
