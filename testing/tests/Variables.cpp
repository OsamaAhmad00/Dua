#include "FileTestCasesRunner.h"

namespace dua
{

TEST(Variables, Variables) {
    FileTestCasesRunner("variables.dua").run();
}

}
